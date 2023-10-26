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

glm::ivec2
set_sprite_custom(entt::registry& r, const entt::entity& e, const std::string& sprite, int tex_unit);

}; // namespace game2d