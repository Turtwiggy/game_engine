#include "pch.hpp"

#include "components.hpp"

#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "particle_helpers.hpp"

namespace game2d {

void
init_particles(entt::registry& r)
{
  auto& particle_c = SINGLE_DefaultParticles::instance;

  ParticleDescriptor pdesc;

  pdesc = {};
  pdesc.size_curve = { { 2, 2 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(0.5 * 1000);
  particle_c.particles[ParticleType::DEFAULT_TRAIL] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 8, 8 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(1.0 * 1000);
  pdesc.random_velocity_bound = 5;
  particle_c.particles[ParticleType::DEFAULT_EXPLODE] = pdesc;

  pdesc = {};
  {
    const float bomb_radius_pixels = 200.0f; // todo: fix this being duplicate
    const float radius_pixels_upper = bomb_radius_pixels;
    pdesc.random_radius_bound_lower = -radius_pixels_upper;
    pdesc.random_radius_bound_upper = radius_pixels_upper;
  }
  pdesc.size_curve = { { 64, 64 }, { 128, 128 }, { 0, 0 } };
  pdesc.start_colour = hex_to_srgb("#de9e41"); // explosion colour
  pdesc.time_to_live_ms = static_cast<int>(0.7f * 1000);
  pdesc.make_darker_based_on_distance_from_center = true;
  pdesc.random_velocity_bound = 50;
  pdesc.velocity_in_dir = true;
  pdesc.linear_scale = false;
  pdesc.sprite = "CIRCLE";
  particle_c.particles[ParticleType::DEATH_SEA_MINE] = pdesc;

  pdesc = {};
  {
    const float radius_pixels = 50.0f; // todo: fix this being duplicate
    const float radius_pixels_upper = radius_pixels;
    pdesc.size_curve = { { 2.0f * radius_pixels_upper, 2.0f * radius_pixels_upper }, { 0, 0 } };
    pdesc.random_radius_bound_lower = -radius_pixels_upper;
    pdesc.random_radius_bound_upper = radius_pixels_upper;
  }
  pdesc.time_to_live_ms = static_cast<int>(0.6 * 1000);
  pdesc.make_darker_based_on_distance_from_center = true;
  pdesc.random_velocity_bound = 50;
  pdesc.velocity_in_dir = true;
  pdesc.start_colour = hex_to_srgb("#de9e41"); // death col
  pdesc.linear_scale = false;
  pdesc.sprite = "CIRCLE";
  particle_c.particles[ParticleType::DEATH_EXPLODER] = pdesc;

  pdesc = {};
  // pdesc.size_curve = { { 0.5 * req.radius_pixels_upper, 0.5 * req.radius_pixels_upper }, { 0, 0 } };
  // pdesc.time_to_live_ms = static_cast<int>(0.2 * 1000);
  // pdesc.start_colour = my_death_sprite_col; // dark red
  // // pdesc.start_colour = hex_to_srgb("#0096ff"); // death col
  // // pdesc.random_velocity_bound = 5;
  // pdesc.velocity_in_dir = true;
  // pdesc.sprite = "SKULL_AND_BONES";
  particle_c.particles[ParticleType::ENEMY_DEATH] = pdesc;

  pdesc = {};
  {
    const float radius_pixels = 16.0f; // todo: fix this being duplicate
    const float radius_pixels_upper = radius_pixels;
    pdesc.size_curve = { { 3.0f * radius_pixels_upper, 3.0f * radius_pixels_upper }, { 0, 0 } };
  }
  pdesc.time_to_live_ms = static_cast<int>(0.8f * 1000);
  pdesc.start_colour = hex_to_srgb("#b1c9c3"); // turret death col
  pdesc.linear_scale = false;
  particle_c.particles[ParticleType::DEFAULT_TURRET_EXPLODE] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 0, 0 }, { 8, 8 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(1.0 * 1000);
  pdesc.start_colour = engine::SRGBColour{ 233, 159, 16, 255 }; // orangeish
  pdesc.random_radius_bound_upper = 16;
  pdesc.random_velocity_bound = 5;
  particle_c.particles[ParticleType::FIRE_PARTICLES] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 0, 0 }, { 4, 4 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(1.0 * 1000);
  pdesc.start_colour = hex_to_srgb("#90D1CA"); // blueish for ice
  pdesc.random_radius_bound_upper = 16;
  pdesc.random_velocity_bound = 5;
  particle_c.particles[ParticleType::ICE_PARTICLES] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 8, 8 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(0.8 * 1000);
  pdesc.start_colour = engine::SRGBColour{ 159, 233, 16, 255 }; // greenish
  pdesc.random_radius_bound_lower = 8;
  pdesc.random_radius_bound_upper = 8;
  pdesc.random_velocity_bound = 50;
  pdesc.velocity_in_dir = true;
  particle_c.particles[ParticleType::VFX_BOOP] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 8, 8 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(0.8 * 1000);
  pdesc.start_colour = hex_to_srgb("#90D1CA"); // blueish for ice
  pdesc.random_radius_bound_lower = 8;
  pdesc.random_radius_bound_upper = 8;
  pdesc.random_velocity_bound = 5;
  pdesc.velocity_in_dir = true;
  particle_c.particles[ParticleType::VFX_ICE_BOOP] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 0, 0 }, { 0, 0 }, { 6, 6 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(1.75 * 1000);
  pdesc.start_colour = hex_to_srgb("#cfc041"); // gold_yellow
  pdesc.random_radius_bound_lower = 0;
  pdesc.random_radius_bound_upper = 0;
  pdesc.random_velocity_bound = 60;
  pdesc.velocity_in_dir = true;
  pdesc.linear_scale = false;
  particle_c.particles[ParticleType::VFX_LEVELUP_OUTER] = pdesc;

  pdesc = {};
  pdesc.size_curve = { { 0, 0 }, { 6, 6 }, { 0, 0 } };
  pdesc.time_to_live_ms = static_cast<int>(2.0 * 1000);
  pdesc.start_colour = hex_to_srgb("#cfc041"); // gold_yellow
  pdesc.random_radius_bound_lower = 32;
  pdesc.random_radius_bound_upper = 32;
  pdesc.random_velocity_bound = 60;
  pdesc.velocity_in_dir = true;
  pdesc.velocity_away = false;
  pdesc.linear_scale = false;
  particle_c.particles[ParticleType::VFX_LEVELUP_INNER] = pdesc;

  // check instantiated.
  assert((int)particle_c.particles.size() == (int)ParticleType::count);
};

} // namespace game2d