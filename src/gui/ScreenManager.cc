#include "ScreenManager.h"
#include "CoreState.h"
#include "game_qs.h"

GUIScreen *mainMenu_create(CoreState *cs, BitFont &font)
{
    SDL_FRect destRect = {0.0f, 0.0f, 640.0f, 480.0f};
    GUIScreen *mainMenu = new GUIScreen(cs, "Main Menu", mainMenuInteractionCallback, destRect);

    SDL_FRect pentominoRect = {20, 20, 100, 20};
    Button *button1 = new Button{0, pentominoRect, "Pentomino C", font};

    SDL_FRect g1MasterRect = {20, 42, 100, 20};
    Button *button2 = new Button{1, g1MasterRect, "G1 Master", font};

    mainMenu->addControlElement(button1);
    mainMenu->addControlElement(button2);

    return mainMenu;
}

void mainMenuInteractionCallback(GUIInteractable &interactable, GUIEvent &event)
{
    if(event.type == mouse_clicked)
    {
        CoreState *cs = interactable.getWindow()->origin;

        switch(interactable.ID)
        {
            default:
                break;

            case 0:
                cs->p1game = qs_game_create(cs, 0, MODE_PENTOMINO, -1);
                cs->p1game->init(cs->p1game);
                break;

            case 1:
                cs->p1game = qs_game_create(cs, 0, MODE_G1_MASTER, -1);
                cs->p1game->init(cs->p1game);
                break;
        }
    }
}
