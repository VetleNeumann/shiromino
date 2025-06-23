#include "video/Screen.h"
#include "Debug.h"
#include "types.h"
#include "SDL3/SDL.h"
#include <iostream>

Shiro::Screen::Screen(const std::string &name, const unsigned w, const unsigned h, const unsigned logicalW, const unsigned logicalH, const float render_scale)
    : name(name)
    , w(w)
    , h(h)
    , logicalW(logicalW)
    , logicalH(logicalH)
    , renderAreaX(0)
    , renderAreaY(0)
    , renderAreaW(0)
    , renderAreaH(0)
    , innerRenderAreaX(0)
    , innerRenderAreaY(0)
    , render_scale(render_scale)
    , window(nullptr)
    , renderer(nullptr)
    , target_tex(nullptr)
{
}

Shiro::Screen::~Screen()
{
    if(renderer)
    {
        SDL_DestroyRenderer(renderer);
    }

    if(window)
    {
        SDL_DestroyWindow(window);
    }
}

bool Shiro::Screen::init(const Settings &settings)
{
    u32 windowFlags = SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow(name.c_str(), w, h, windowFlags);
    if(window == nullptr)
    {
        log_err("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if(renderer == nullptr)
    {
        log_err("SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }

    if(SDL_SetRenderVSync(renderer, settings.vsync ? 1 : 0) < 0)
    {
        log_err("SDL_SetRenderVSync: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return false;
    }

    if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0)
    {
        log_err("SDL_SetRenderDrawBlendMode: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return false;
    }

    SDL_SetWindowMinimumSize(window, 640, 480);
    if(settings.fullscreen)
    {
        if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) < 0)
        {
            log_err("SDL_SetWindowFullscreen: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }

    return true;
}

void Shiro::Screen::updateRenderAreaPosition()
{
    float aspect = float(w) / float(h);
    float aspectDefault = float(logicalW) / float(logicalH);

    renderAreaX = 0;
    renderAreaY = 0;
    renderAreaW = w;
    renderAreaH = h;

    if(aspect > aspectDefault) // extra width
    {
        renderAreaW = aspectDefault * float(h);
        renderAreaX = (w - renderAreaW) / 2;
    }
    else if(aspect < aspectDefault) // extra height
    {
        renderAreaH = float(w) / aspectDefault;
        renderAreaY = (h - renderAreaH) / 2;
    }

    innerRenderAreaX = (w - static_cast<int>(float(logicalW) * render_scale)) / 2;
    innerRenderAreaY = (h - static_cast<int>(float(logicalH) * render_scale)) / 2;
}