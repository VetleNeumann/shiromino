#pragma once
#include <cstdint>

namespace Shiro {
struct KeyFlags
{
    bool operator==(const KeyFlags &cmp) const;
    bool operator!=(const KeyFlags &cmp) const;

    std::uint8_t left;
    std::uint8_t right;
    std::uint8_t up;
    std::uint8_t down;
    std::uint8_t a;
    std::uint8_t b;
    std::uint8_t c;
    std::uint8_t d;
    std::uint8_t start;
    std::uint8_t escape;
};
} // namespace Shiro