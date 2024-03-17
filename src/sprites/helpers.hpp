#pragma once

// my libs
#include "components.hpp"

// std libs
#include <string>
#include <vector>

namespace game2d {

void
load_sprites(SINGLE_Animations& anims, const std::string path);

std::pair<Spritesheet, SpriteAnimation>
find_animation(const SINGLE_Animations& anims, const std::string name);

void
set_sprite(entt::registry& r, const entt::entity& e, const std::string& sprite);

// const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
// const auto tex_unit = search_for_texture_unit_by_path(ri, "bargame");
void
set_sprite_custom(entt::registry& r, const entt::entity& e, const std::string& sprite, int tex_unit);

}; // namespace game2d