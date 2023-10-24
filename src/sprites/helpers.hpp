#pragma once

// my libs
#include "components.hpp"

// std libs
#include <string>
#include <vector>

namespace game2d {

void
load_sprites(SINGLE_Animations& anims, const std::string path);

SpriteAnimation
find_animation(const SINGLE_Animations& anims, const std::string name);

void
set_sprite(entt::registry& r, const entt::entity& e, const std::string& sprite);

}; // namespace game2d