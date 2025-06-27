#include <functional>
#include <memory>
#include <utility>

#include "GridCanvas.h"
#include "SDL3/SDL.h"
#include "gfx_helpers.h"
#include "gui/GUI.h"

GridCanvas::GridCanvas(int ID, Shiro::Grid *cells, BindableInt &paletteVar, SDL_Texture *paletteTex, unsigned int cellW, unsigned int cellH,
                       SDL_FRect relativeDestRect)
    : cells(cells), paletteTex(paletteTex), cellW(cellW), cellH(cellH), paletteVar(paletteVar)
{
    this->relativeDestRect = relativeDestRect;
    this->ID = ID;

    readPaletteSelection(&paletteVar);
    paletteSize = (unsigned)std::get<1>(paletteVar.getRange());

    std::function<void(BindableVariable *)> membFunc = [this](BindableVariable *bv) { this->readPaletteSelection(bv); };

    std::unique_ptr<VariableObserver> vob{(VariableObserver *)(new MemberVariableObserver{membFunc})};

    paletteVar.addObserver(vob);

    clipboard = NULL;
    selection = false;
    clipboardMoveMode = false;
    gridLinesShown = false;
    cursorShown = false;

    enabled = true;
    selected = false;
    canHoldKeyboardFocus = true;
    hasDefaultKeyboardFocus = true;
    hasKeyboardFocus = false;

    updateDisplayStringPVs = false;
}

void GridCanvas::draw()
{
    this->prepareRenderTarget();

    GUIDrawBorder(relativeDestRect, 1, Shiro::GUI::RGBA_DEFAULT);

    for(int i = 0; i < int(cells->getWidth()); i++)
    {
        for(int j = 0; j < int(cells->getHeight()); j++)
        {
            std::vector<unsigned int> paletteList;
            GUIVirtualPoint point = {i, j};

            int val = getCell(point);

            if(clipboardMoveMode && clipboard)
            {
                if(i >= cellUnderMouse.x && j >= cellUnderMouse.y && i < cellUnderMouse.x + int(clipboard->getWidth()) &&
                   j < cellUnderMouse.y + int(clipboard->getHeight()))
                {
                    int x = i - cellUnderMouse.x;
                    int y = j - cellUnderMouse.y;
                    val = clipboard->getCell(x, y);
                }
            }

            int destX = relativeDestRect.x + (i * cellW);
            int destY = relativeDestRect.y + (j * cellH);

            auto src = make_frect(0, 0, cellW, cellH);
            const auto dest = make_frect(destX, destY, cellW, cellH);

            if(!paletteValMap.empty())
            {
                fillCellPaletteListFromMappedVal(val, paletteList);

                for(auto m : paletteList)
                {
                    src.x = m * cellW;

                    if(paletteTex)
                    {
                        SDL_RenderTexture(guiSDLRenderer, paletteTex, &src, &dest);
                    }
                }
            }
            else
            {
                val--;
                if(val >= 0)
                {
                    src.x = val * cellW;

                    if(paletteTex)
                    {
                        SDL_RenderTexture(guiSDLRenderer, paletteTex, &src, &dest);
                    }
                }
            }
        }
    }

    if(selection)
    {
        int lesserX = selectionVertex1.x;
        int lesserY = selectionVertex1.y;
        int greaterX = selectionVertex2.x;
        int greaterY = selectionVertex2.y;

        if(lesserX > greaterX)
        {
            int swp = lesserX;
            lesserX = greaterX;
            greaterX = swp;
        }
        const int width = greaterX - lesserX + 1;

        if(lesserY > greaterY)
        {
            int swp = lesserY;
            lesserY = greaterY;
            greaterY = swp;
        }
        const int height = greaterY - lesserY + 1;

        Shiro::GridRect rect = {lesserX, lesserY, static_cast<std::size_t>(width), static_cast<std::size_t>(height)};

        const auto selectionRect = SDL_FRect{
            relativeDestRect.x + (rect.x * (float)cellW), relativeDestRect.y + (rect.y * (float)cellH), rect.width * (float)cellW, rect.height * (float)cellH};

        Shiro::GUI::rgba_t v = 0x9090FF9F;

        SDL_SetRenderDrawColor(guiSDLRenderer, rgba_R(v), rgba_G(v), rgba_B(v), rgba_A(v));
        SDL_RenderFillRect(guiSDLRenderer, &selectionRect);
        SDL_SetRenderDrawColor(guiSDLRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }

    if(cursorShown && !editInProgress)
    {
        int cursorX = relativeDestRect.x + (cellUnderMouse.x * cellW);
        int cursorY = relativeDestRect.y + (cellUnderMouse.y * cellH);

        const SDL_FRect cursorRect = make_frect(cursorX, cursorY, (int)cellW, (int)cellH);

        Shiro::GUI::rgba_t v = 0xEFEFEF9F;

        SDL_SetRenderDrawColor(guiSDLRenderer, rgba_R(v), rgba_G(v), rgba_B(v), rgba_A(v));
        SDL_RenderFillRect(guiSDLRenderer, &cursorRect);
        SDL_SetRenderDrawColor(guiSDLRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    }
}

void GridCanvas::handleEvent(GUIEvent &event)
{
    switch(event.type)
    {
        case mouse_hovered_onto:
            gridLinesShown = true;
            cursorShown = true;
            break;

        case mouse_hovered_off:
            gridLinesShown = false;
            cursorShown = false;
            editInProgress = false;
            break;

        case mouse_clicked:
            this->mouseClicked(event.mouseClickedEvent->x, event.mouseClickedEvent->y, event.mouseClickedEvent->button);
            break;

        case mouse_released:
            this->mouseReleased(event.mouseReleasedEvent->x, event.mouseReleasedEvent->y, event.mouseReleasedEvent->button);
            break;

        case mouse_dragged:
            this->mouseDragged(event.mouseDraggedEvent->x, event.mouseDraggedEvent->y, event.mouseDraggedEvent->button);
            break;

        case mouse_moved:
            this->mouseMoved(event.mouseMovedEvent->x, event.mouseMovedEvent->y);
            break;

        case key_pressed:
            this->keyPressed(event.keyPressedEvent->key);
            break;

        default:
            break;
    }
}

void GridCanvas::mouseMoved(int x, int y) { cellUnderMouse = xyToCell(x, y); }

void GridCanvas::mouseClicked(int x, int y, Uint8 button)
{
    if(clipboardMoveMode)
    {
        if(button == SDL_BUTTON_LEFT)
        {
            makeBackup();
            pasteSelection();
            clipboardMoveMode = false;
        }

        return;
    }

    makeBackup();

    if(button == SDL_BUTTON_LEFT)
    {
        if(SDL_GetModState() & SDL_KMOD_SHIFT)
        {
            selectionVertex1 = selectionVertex2 = cellUnderMouse;
            selection = true;
        }
        else
        {
            GUIVirtualPoint point = xyToCell(x, y);
            fillCell(point);
            selection = false;
            editInProgress = true;
        }
    }
    else if(button == SDL_BUTTON_RIGHT)
    {
        GUIVirtualPoint point = xyToCell(x, y);
        eraseCell(point);
        selection = false;
        editInProgress = true;
    }
}

void GridCanvas::mouseDragged(int x, int y, Uint8 button)
{
    if(!editInProgress)
    {
        makeBackup();
        editInProgress = true;
    }

    cellUnderMouse = xyToCell(x, y);

    if(button == SDL_BUTTON_LEFT)
    {
        if(SDL_GetModState() & SDL_KMOD_SHIFT)
        {
            selectionVertex2 = cellUnderMouse;
        }
        else
        {
            GUIVirtualPoint point = xyToCell(x, y);
            fillCell(point);
        }
    }
    else if(button == SDL_BUTTON_RIGHT)
    {
        GUIVirtualPoint point = xyToCell(x, y);
        eraseCell(point);
    }
}

void GridCanvas::mouseReleased(int, int, Uint8) { editInProgress = false; }

void GridCanvas::keyPressed(SDL_Keycode kc)
{
    unsigned num = 0;
    bool pressedNumber = false;

    switch(kc)
    {
        case SDLK_A:
            if(SDL_GetModState() & SDL_KMOD_CTRL)
            {
                selectionVertex1.x = 0;
                selectionVertex1.y = 0;
                selectionVertex2.x = (int)cells->getWidth() - 1;
                selectionVertex2.y = (int)cells->getHeight() - 1;
                selection = true;
            }

            break;

        case SDLK_X:
            if(SDL_GetModState() & SDL_KMOD_CTRL)
            {
                cutSelection();
                selection = false;
            }

            break;

        case SDLK_C:
            if(SDL_GetModState() & SDL_KMOD_CTRL)
            {
                copySelection();
            }

            break;

        case SDLK_V:
            if(SDL_GetModState() & SDL_KMOD_CTRL)
            {
                if(clipboard != NULL)
                {
                    clipboardMoveMode = true;
                    selection = false;
                }
            }

            break;

        case SDLK_Z:
            if(SDL_GetModState() & SDL_KMOD_CTRL)
            {
                undo();
            }

            break;

        case SDLK_Y:
            if(SDL_GetModState() & SDL_KMOD_CTRL)
            {
                redo();
            }

            break;

        case SDLK_1:
            num = 0;
            pressedNumber = true;
            break;

        case SDLK_2:
            num = 1;
            pressedNumber = true;
            break;

        case SDLK_3:
            num = 2;
            pressedNumber = true;
            break;

        case SDLK_4:
            num = 3;
            pressedNumber = true;
            break;

        case SDLK_5:
            num = 4;
            pressedNumber = true;
            break;

        case SDLK_6:
            num = 5;
            pressedNumber = true;
            break;

        case SDLK_7:
            num = 6;
            pressedNumber = true;
            break;

        case SDLK_8:
            num = 7;
            pressedNumber = true;
            break;

        case SDLK_9:
            num = 8;
            pressedNumber = true;
            break;

        case SDLK_0:
            num = 9;
            pressedNumber = true;
            break;

        default:
            break;
    }

    if(pressedNumber && num < paletteSize)
    {
        if(selection)
        {
            int lesserX = selectionVertex1.x;
            int lesserY = selectionVertex1.y;
            int greaterX = selectionVertex2.x;
            int greaterY = selectionVertex2.y;

            if(lesserX > greaterX)
            {
                int swp = lesserX;
                lesserX = greaterX;
                greaterX = swp;
            }
            const int width = greaterX - lesserX + 1;

            if(lesserY > greaterY)
            {
                int swp = lesserY;
                lesserY = greaterY;
                greaterY = swp;
            }
            const int height = greaterY - lesserY + 1;

            Shiro::GridRect selectionRect = {lesserX, lesserY, static_cast<std::size_t>(width), static_cast<std::size_t>(height)};

            if(paletteValMap.size() <= (std::size_t)num + 1)
            {
                makeBackup();

                cells->fill(selectionRect, num + 1);
                selection = false;
            }
            else if(!paletteValMap[(std::size_t)num + 1].isFlag)
            {
                makeBackup();

                cells->fill(selectionRect, paletteValMap[(std::size_t)num + 1].mappedVal);
                selection = false;
            }
        }
        else
        {
            paletteVar.setInt(static_cast<int64_t>(num));
        }
    }
}

void GridCanvas::makeBackup()
{
    undoBuffer.push_back(new Shiro::Grid(*cells));

    for(auto r : redoBuffer)
    {
        delete r;
    }

    redoBuffer.clear();
}

void GridCanvas::undo()
{
    if(undoBuffer.empty())
    {
        return;
    }

    redoBuffer.push_back(new Shiro::Grid(*cells));
    cells = undoBuffer.back();
    undoBuffer.pop_back();
}

void GridCanvas::redo()
{
    if(redoBuffer.empty())
    {
        return;
    }

    undoBuffer.push_back(new Shiro::Grid(*cells));
    cells = redoBuffer.back();
    redoBuffer.pop_back();
}

void GridCanvas::clearUndo()
{
    for(auto u : undoBuffer)
    {
        delete u;
    }

    for(auto r : redoBuffer)
    {
        delete r;
    }

    undoBuffer.clear();
    redoBuffer.clear();
}

void GridCanvas::copySelection()
{
    if(clipboard)
    {
        delete clipboard;
    }

    int lesserX = selectionVertex1.x;
    int lesserY = selectionVertex1.y;
    int greaterX = selectionVertex2.x;
    int greaterY = selectionVertex2.y;

    if(lesserX > greaterX)
    {
        int swp = lesserX;
        lesserX = greaterX;
        greaterX = swp;
    }
    const int width = greaterX - lesserX + 1;

    if(lesserY > greaterY)
    {
        int swp = lesserY;
        lesserY = greaterY;
        greaterY = swp;
    }
    const int height = greaterY - lesserY + 1;

    Shiro::GridRect selectionRect = {lesserX, lesserY, static_cast<std::size_t>(width), static_cast<std::size_t>(height)};
    clipboard = new Shiro::Grid(*cells, selectionRect);
}

void GridCanvas::cutSelection()
{
    if(clipboard)
    {
        delete clipboard;
    }

    int lesserX = selectionVertex1.x;
    int lesserY = selectionVertex1.y;
    int greaterX = selectionVertex2.x;
    int greaterY = selectionVertex2.y;

    if(lesserX > greaterX)
    {
        int swp = lesserX;
        lesserX = greaterX;
        greaterX = swp;
    }
    const int width = greaterX - lesserX + 1;

    if(lesserY > greaterY)
    {
        int swp = lesserY;
        lesserY = greaterY;
        greaterY = swp;
    }
    const int height = greaterY - lesserY + 1;

    Shiro::GridRect selectionRect = {lesserX, lesserY, static_cast<std::size_t>(width), static_cast<std::size_t>(height)};
    clipboard = new Shiro::Grid(*cells, selectionRect);

    if(paletteValMap.size() == 0)
    {
        cells->fill(selectionRect, 0);
    }
    else
    {
        cells->fill(selectionRect, paletteValMap[0].mappedVal);
    }
}

void GridCanvas::pasteSelection()
{
    if(!clipboard)
    {
        return;
    }

    Shiro::GridRect dest = {cellUnderMouse.x, cellUnderMouse.y, clipboard->getWidth(), clipboard->getHeight()};
    cells->copyRect(*clipboard, {0, 0, clipboard->getWidth(), clipboard->getHeight()}, dest);
}

void GridCanvas::eraseCell(GUIVirtualPoint &point)
{
    if(paletteValMap.size() == 0)
    {
        cells->cell(point.x, point.y) = 0;
    }
    else
    {
        cells->cell(point.x, point.y) = paletteValMap[0].mappedVal;
    }
}

void GridCanvas::fillCell(GUIVirtualPoint &point)
{
    if(paletteValMap.size() <= paletteSelection)
    {
        cells->cell(point.x, point.y) = paletteSelection + 1;
    }
    else
    {
        if(paletteValMap[(std::size_t)paletteSelection + 1].isFlag && (getCell(point) != paletteValMap[0].mappedVal))
        // only allow xor if the cell isn't empty
        {
            cells->xorCell(point.x, point.y, paletteValMap[(size_t)paletteSelection + 1].mappedVal);
        }
        else
        {
            cells->cell(point.x, point.y) = paletteValMap[(size_t)paletteSelection + 1].mappedVal;
        }
    }
}

int GridCanvas::getCell(GUIVirtualPoint &point) { return cells->getCell(point.x, point.y); }

void GridCanvas::fillCellPaletteListFromMappedVal(int mappedVal, std::vector<unsigned int> &paletteList)
{
    paletteList.clear();

    int baseVal = -1;

    for(unsigned int i = 1; i < paletteValMap.size(); i++)
    {
        paletteMapEntry m = paletteValMap[i];
        if(m.isFlag)
        {
            if(mappedVal & m.mappedVal)
            {
                paletteList.push_back(i - 1);
            }
        }
        else
        {
            if(mappedVal & m.mappedVal)
            {
                baseVal = (int)(i);
            }
        }
    }

    if(baseVal >= 0)
    {
        paletteList.insert(paletteList.begin(), baseVal);
    }
}