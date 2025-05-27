#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
setup_floor_mask_update(entt::registry& r);

void
setup_water_update(entt::registry& r);

// FBO: Render sprites in to this fbo with linear colour
void
setup_linear_main_update(entt::registry& r);

void
setup_sprites_to_outline_update(entt::registry& r);
void
setup_outline_update(entt::registry& r);

void
setup_sprites_with_shield_update(entt::registry& r);
void
setup_shine_update(entt::registry& r);

void
setup_lighting_emitters_and_occluders_update(entt::registry& r);

void
setup_voronoi_seed_update(entt::registry& r);
void
setup_jump_flood_pass(entt::registry& r);
void
setup_voronoi_distance_field_update(entt::registry& r);

// mix scene and lighting textures
void
setup_mix_lighting_and_scene_update(entt::registry& r);

void
setup_crt_effect_update(entt::registry& r);

// FBO: gaussian blur.
// void
// setup_gaussian_blur_update(entt::registry& r);
// void
// setup_bloom_update(entt::registry& r);

} // namespace game2d