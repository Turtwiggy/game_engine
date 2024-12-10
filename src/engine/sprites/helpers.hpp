#pragma once

// my libs
#include "components.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

#include <string>

namespace game2d {

void
load_sprites(SINGLE_Animations& anims, const std::string& path);

std::pair<Spritesheet, SpriteAnimation>
find_animation(const SINGLE_Animations& anims, const std::string& name);

void
set_sprite(entt::registry& r, const entt::entity e, const std::string& sprite);

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite);

}; // namespace game2d