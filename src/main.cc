#include "CoreState.h"
#include "SDL3/SDL.h"
#include "SDL3_image/SDL_image.h"
#include "SDL3_mixer/SDL_mixer.h"
#include "random.h"
#include <cstdlib>
#include <iostream>

/*
 * Frees several SDL-related resources.
 */
static void destroy()
{
    Mix_Quit();
    SDL_Quit();
}
/*
 * Initializes several SDL-related resources.
 */
void initialize(const Shiro::Settings &settings)
{
    if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false)
    {
        std::cerr << "SDL_Init: Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) == false)
    {
        std::cerr << "SDL_InitSubSystem: Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if(SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_HIGH) == false)
    {
        std::cerr << "Failed to set high thread priority; continuing without changing thread priority" << std::endl;
    }
    // if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    // {
    //     std::cerr << "IMG_Init: Failed to initialize PNG support: " << SDL_GetError() << std::endl;
    //     std::exit(EXIT_FAILURE);
    // }
    if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
    {
        std::cerr << "Mix_Init: Failed to initialize OGG support: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    SDL_AudioSpec desiredSpec;
    desiredSpec.freq = settings.samplingRate;
    desiredSpec.format = MIX_DEFAULT_FORMAT;
    desiredSpec.channels = 32;
    if(Mix_OpenAudio(0, &desiredSpec) == -1)
    {
        std::cerr << "Mix_OpenAudio: Error: " << SDL_GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    g123_seeds_init();
    std::srand((unsigned int)std::time(0));
    std::atexit(destroy);
}
/*
 * The game's entry point.
 */
int main(int argc, char *argv[])
{
    int returnCode = EXIT_FAILURE;
    Shiro::Settings settings;
    if(settings.init(argc, argv))
    {
        std::cerr << "Configuration path: " << settings.configurationPath.string() << std::endl;
        std::cerr << "Cache path: " << settings.cachePath.string() << std::endl;
        std::cerr << "Share path: " << settings.sharePath.string() << std::endl;
        initialize(settings);
        CoreState cs(settings);
        if(cs.init())
        {
            returnCode = EXIT_SUCCESS;
            cs.run();
        }
    }
    return returnCode;
}