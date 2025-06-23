#include "video/Render.h"
#include "video/Screen.h"

using namespace Shiro;

namespace Shiro {
int RenderCopy(const Screen &screen, SDL_Texture *tex, const SDL_FRect *srcrect, SDL_FRect *dstrect)
{
    if(screen.window == nullptr || screen.renderer == nullptr || tex == nullptr)
    {
        return -1;
    }

    float scaleX;
    float scaleY;
    SDL_GetRenderScale(screen.renderer, &scaleX, &scaleY);

    if(SDL_GetRenderTarget(screen.renderer) != nullptr)
    {
        if(dstrect != nullptr)
        {

            const SDL_FRect scaledRect = {
                static_cast<float>(dstrect->x) * scaleX,
                static_cast<float>(dstrect->y) * scaleY,
                static_cast<float>(dstrect->w) * scaleX,
                static_cast<float>(dstrect->h) * scaleY,
            };

            return SDL_RenderTexture(screen.renderer, tex, srcrect, &scaledRect);
        }
        else
        {
            return SDL_RenderTexture(screen.renderer, tex, srcrect, dstrect);
        }
    }

    if(dstrect == nullptr)
    {
        SDL_FRect rect = {screen.renderAreaX, screen.renderAreaY, screen.renderAreaW, screen.renderAreaH};

        SDL_SetRenderScale(screen.renderer, 1.0, 1.0);
        int rc = SDL_RenderTexture(screen.renderer, tex, srcrect, &rect);
        SDL_SetRenderScale(screen.renderer, scaleX, scaleY);

        return rc;
    }

    int scaledRenderAreaX = static_cast<int>(float(screen.innerRenderAreaX) / scaleX);
    int scaledRenderAreaY = static_cast<int>(float(screen.innerRenderAreaY) / scaleY);

    SDL_FRect dst = {dstrect->x, dstrect->y, dstrect->w, dstrect->h};

    dst.x += scaledRenderAreaX;
    dst.y += scaledRenderAreaY;

    return SDL_RenderTexture(screen.renderer, tex, srcrect, &dst);
}

int RenderCopyOuter(const Screen &screen, SDL_Texture *tex, const SDL_FRect *srcrect, SDL_FRect *dstrect, Render::Alignment alignment)
{
    if(screen.window == nullptr || screen.renderer == nullptr || tex == nullptr)
    {
        return -1;
    }

    float scaleX;
    float scaleY;
    SDL_GetRenderScale(screen.renderer, &scaleX, &scaleY);

    if(SDL_GetRenderTarget(screen.renderer) != nullptr)
    {
        if(dstrect != nullptr)
        {
            SDL_FRect scaledRect = {
                static_cast<float>(dstrect->x) * scaleX,
                static_cast<float>(dstrect->y) * scaleY,
                static_cast<float>(dstrect->w) * scaleX,
                static_cast<float>(dstrect->h) * scaleY,
            };

            return SDL_RenderTexture(screen.renderer, tex, srcrect, &scaledRect);
        }
        else
        {
            return SDL_RenderTexture(screen.renderer, tex, srcrect, dstrect);
        }
    }

    if(dstrect == nullptr)
    {
        SDL_FRect rect = {screen.renderAreaX, screen.renderAreaY, screen.renderAreaW, screen.renderAreaH};

        SDL_SetRenderScale(screen.renderer, 1.0, 1.0);
        int rc = SDL_RenderTexture(screen.renderer, tex, srcrect, &rect);
        SDL_SetRenderScale(screen.renderer, scaleX, scaleY);

        return rc;
    }
    else
    {
        SDL_FRect scaledRect = {
            static_cast<float>(dstrect->x) * scaleX,
            static_cast<float>(dstrect->y) * scaleY,
            static_cast<float>(dstrect->w) * scaleX,
            static_cast<float>(dstrect->h) * scaleY,
        };

        int xDelta = 0;
        int yDelta = 0;

        switch(alignment)
        {
            case Render::Alignment::topLeft:
                xDelta = screen.renderAreaX;
                yDelta = screen.renderAreaY;
                break;

            case Render::Alignment::bottomLeft:
                xDelta = screen.renderAreaX;
                yDelta = screen.renderAreaY + screen.renderAreaH;
                break;

            case Render::Alignment::topRight:
                xDelta = screen.renderAreaX + screen.renderAreaW;
                yDelta = screen.renderAreaY;
                break;

            case Render::Alignment::bottomRight:
                xDelta = screen.renderAreaX + screen.renderAreaW;
                yDelta = screen.renderAreaY + screen.renderAreaH;
                break;

            default:
                break;
        }

        SDL_FRect dst = {scaledRect.x + xDelta, scaledRect.y + yDelta, scaledRect.w, scaledRect.h};

        SDL_SetRenderScale(screen.renderer, 1.0, 1.0);
        int rc = SDL_RenderTexture(screen.renderer, tex, srcrect, &dst);
        SDL_SetRenderScale(screen.renderer, scaleX, scaleY);

        return rc;
    }
}

int RenderFillRect(const Screen &screen, SDL_FRect *rect)
{
    if(screen.window == nullptr || screen.renderer == nullptr)
    {
        return -1;
    }

    float scaleX;
    float scaleY;
    SDL_GetRenderScale(screen.renderer, &scaleX, &scaleY);

    if(SDL_GetRenderTarget(screen.renderer) != nullptr)
    {
        if(rect != nullptr)
        {
            SDL_FRect scaledRect = {
                static_cast<float>(rect->x) * scaleX,
                static_cast<float>(rect->y) * scaleY,
                static_cast<float>(rect->w) * scaleX,
                static_cast<float>(rect->h) * scaleY,
            };

            return SDL_RenderFillRect(screen.renderer, &scaledRect);
        }
        else
        {
            return SDL_RenderFillRect(screen.renderer, rect);
        }
    }

    if(rect == nullptr)
    {
        SDL_FRect rect_ = {screen.renderAreaX, screen.renderAreaY, screen.renderAreaW, screen.renderAreaH};

        SDL_SetRenderScale(screen.renderer, 1.0, 1.0);
        int rc = SDL_RenderFillRect(screen.renderer, &rect_);
        SDL_SetRenderScale(screen.renderer, scaleX, scaleY);

        return rc;
    }

    int scaledRenderAreaX = static_cast<int>(float(screen.innerRenderAreaX) / scaleX);
    int scaledRenderAreaY = static_cast<int>(float(screen.innerRenderAreaY) / scaleY);

    SDL_FRect dst = {rect->x, rect->y, rect->w, rect->h};

    dst.x += scaledRenderAreaX;
    dst.y += scaledRenderAreaY;

    return SDL_RenderFillRect(screen.renderer, &dst);
}

int RenderFillRectOuter(const Screen &screen, SDL_FRect *rect, Render::Alignment alignment)
{
    if(screen.window == nullptr || screen.renderer == nullptr)
    {
        return -1;
    }

    float scaleX;
    float scaleY;
    SDL_GetRenderScale(screen.renderer, &scaleX, &scaleY);

    if(SDL_GetRenderTarget(screen.renderer) != nullptr)
    {
        if(rect != nullptr)
        {
            SDL_FRect scaledRect = {
                static_cast<float>(rect->x) * scaleX,
                static_cast<float>(rect->y) * scaleY,
                static_cast<float>(rect->w) * scaleX,
                static_cast<float>(rect->h) * scaleY,
            };

            return SDL_RenderFillRect(screen.renderer, &scaledRect);
        }
        else
        {
            return SDL_RenderFillRect(screen.renderer, rect);
        }
    }

    if(rect == nullptr)
    {
        SDL_FRect rect_ = {screen.renderAreaX, screen.renderAreaY, screen.renderAreaW, screen.renderAreaH};

        SDL_SetRenderScale(screen.renderer, 1.0, 1.0);
        int rc = SDL_RenderFillRect(screen.renderer, &rect_);
        SDL_SetRenderScale(screen.renderer, scaleX, scaleY);

        return rc;
    }
    else
    {
        SDL_FRect scaledRect = {
            static_cast<float>(rect->x) * scaleX,
            static_cast<float>(rect->y) * scaleY,
            static_cast<float>(rect->w) * scaleX,
            static_cast<float>(rect->h) * scaleY,
        };

        int xDelta = 0;
        int yDelta = 0;

        switch(alignment)
        {
            case Render::Alignment::topLeft:
                xDelta = screen.renderAreaX;
                yDelta = screen.renderAreaY;
                break;

            case Render::Alignment::bottomLeft:
                xDelta = screen.renderAreaX;
                yDelta = screen.renderAreaY + screen.renderAreaH;
                break;

            case Render::Alignment::topRight:
                xDelta = screen.renderAreaX + screen.renderAreaW;
                yDelta = screen.renderAreaY;
                break;

            case Render::Alignment::bottomRight:
                xDelta = screen.renderAreaX + screen.renderAreaW;
                yDelta = screen.renderAreaY + screen.renderAreaH;
                break;

            default:
                break;
        }

        SDL_FRect dst = {scaledRect.x + xDelta, scaledRect.y + yDelta, scaledRect.w, scaledRect.h};

        SDL_SetRenderScale(screen.renderer, 1.0, 1.0);
        int rc = SDL_RenderFillRect(screen.renderer, &dst);
        SDL_SetRenderScale(screen.renderer, scaleX, scaleY);

        return rc;
    }
}
} // namespace Shiro