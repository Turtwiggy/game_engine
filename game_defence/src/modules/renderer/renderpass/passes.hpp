#pragma once

#include <entt/entt.hpp>

namespace game2d {

// FBO: Render sprites in to this fbo with linear colour
void
setup_linear_main_update(entt::registry& r);

void
setup_lighting_main_update(entt::registry& r);

// mix scene and lighting textures
void
setup_mix_lighting_and_scene_update(entt::registry& r);

// FBO: gaussian blur.
void
setup_gaussian_blur_update(entt::registry& r);

void
setup_bloom_update(entt::registry& r);

} // namespace game2d