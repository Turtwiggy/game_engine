#pragma once

// my libs
#include "components.hpp"
#include "engine/opengl/texture.hpp"

#include <entt/fwd.hpp>

#include <string>

namespace game2d {

void
load_sprites(SINGLE_Animations& anims, const engine::Texture& texture);

std::pair<int, int>
find_animation(const SINGLE_Animations& anims, const std::string& name);

void
set_sprite(entt::registry& r, const entt::entity e, const std::string& sprite);

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite);

}; // namespace game2d