#pragma once
#include "SDL3/SDL.h"
#include "video/Screen.h"

using namespace Shiro;

namespace Shiro {
namespace Render {
enum class Alignment { topLeft, bottomLeft, topRight, bottomRight };
}

int RenderCopy(const Screen &screen, SDL_Texture *tex, const SDL_FRect *srcrect, SDL_FRect *dstrect);
int RenderCopyOuter(const Screen &screen, SDL_Texture *tex, const SDL_FRect *srcrect, SDL_FRect *dstrect, Render::Alignment alignment);
int RenderFillRect(const Screen &screen, SDL_FRect *rect);
int RenderFillRectOuter(const Screen &screen, SDL_FRect *rect, Render::Alignment alignment);
} // namespace Shiro