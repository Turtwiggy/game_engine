#pragma once

// my libs
#include "components.hpp"

// std libs
#include <string>
#include <vector>

namespace game2d {

void
load_sprites(std::vector<SpriteAnimation>& sprites, const std::string path);

SpriteAnimation
find_animation(const std::vector<SpriteAnimation>& sprites, const std::string name);

}; // namespace game2d