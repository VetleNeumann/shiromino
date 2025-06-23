#include "OS.h"
#include "SDL3/SDL.h"
#include "definitions.h"
#include <filesystem>

namespace fs = std::filesystem;

const fs::path &Shiro::OS::getBasePath()
{
    static fs::path basePath;

    if(basePath.empty())
    {
#ifdef APPIMAGE_BASE_PATH
        char *const appImageFilename = getenv("APPIMAGE");
        if(appImageFilename == NULL)
        {
            throw std::logic_error("Failed to get AppImage base path.");
        }
        else
        {
            basePath = fs::path(appImageFilename).remove_filename();
        }
#else
        const char *basePathCStr = SDL_GetBasePath();
        if(basePathCStr == NULL)
        {
            throw std::logic_error("Failed to get SDL base path.");
        }
        basePath = fs::path(basePathCStr);
#endif
    }

    return basePath;
}
