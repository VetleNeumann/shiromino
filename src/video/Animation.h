#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>

#include "SDL3/SDL.h"
#include "asset/Asset.h"
#include "video/Gfx.h"
#include "video/Screen.h"

namespace Shiro {
struct AnimationGraphic : public Graphic
{
    AnimationGraphic() = delete;

    AnimationGraphic(const Screen &screen, SDL_Texture *const frame, const int x, const int y, const std::uint32_t rgbaMod);

    void draw() const;

    const Screen &screen;
    SDL_Texture *frame;
    const int x;
    const int y;
    const std::uint32_t rgbaMod;
};

class AnimationEntity : public Entity
{
  public:
    DEFINE_ENTITY_PUSH(AnimationEntity)

    AnimationEntity() = delete;

    AnimationEntity(const Screen &screen, AssetManager &assetMgr, const std::filesystem::path &frames, const std::size_t layerNum, const int x, const int y,
                    const std::size_t numFrames, const std::size_t frameMultiplier, const std::uint32_t rgbaMod);

    bool update(Layers &layers);

  private:
    AssetManager &assetMgr;
    const std::filesystem::path frames;
    const std::size_t layerNum;
    std::size_t counter;
    const std::size_t numFrames;
    const std::size_t frameMultiplier;
    const std::shared_ptr<AnimationGraphic> graphic;
};
} // namespace Shiro
