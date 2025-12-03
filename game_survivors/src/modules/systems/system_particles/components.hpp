#pragma once

#include "engine/colour/colour.hpp"
#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct ParticleDescriptor
{
  int time_to_live_ms = 1 * 1000;

  // size curve
  std::vector<glm::vec2> size_curve{ { 0, 0 }, { 16, 16 }, { 0, 0 } };
  bool linear_scale = true;

  bool random_rotation = false;

  bool fade = true;
  engine::SRGBColour start_colour{ 1.0f, 1.0f, 1.0f, 1.0f };

  // spawn in a circle around the position?
  float random_radius_bound_lower = 0;
  float random_radius_bound_upper = 0;

  // if 0, no random velocity,
  // otherwise generates velocity in [-val, val]
  float random_velocity_bound = 0;

  // moves the velocity away from the center of the particle
  bool velocity_in_dir = false;
  bool velocity_away = true;

  bool make_darker_based_on_distance_from_center = false;
  // engine::SRGBColour end_colour;  // not implemented

  std::string sprite = "EMPTY";
};

struct ParticleEmitter
{
  entt::entity parent = entt::null;
  glm::vec2 velocity{ 0.0f, 0.0f };

  float start_size = 6.0f;
  float end_size = 2.0f;
};

// related components:
// ScaleOverTimeComponent
// SpriteAnimationComponent
// EntityTimedLifecycle
// TransformComponent
// CooldownComponent

// used by non-physics body components
struct VelocityComponent
{
  float x = 0;
  float y = 0;
};

struct ScaleOverTimeComponent
{
  float timer = 0.0f;
  float seconds_until_complete = 3.0f;
  std::vector<glm::vec2> size_curve{ { 16, 16 }, { 0, 0 } };
  bool linear = true;
};

enum class ParticleType
{
  DEFAULT_TRAIL = 0,
  DEFAULT_EXPLODE,
  DEATH_SEA_MINE,
  DEATH_EXPLODER,
  ENEMY_DEATH,
  DEFAULT_TURRET_EXPLODE,
  FIRE_PARTICLES,
  ICE_PARTICLES,
  VFX_BOOP,
  VFX_ICE_BOOP,
  VFX_LEVELUP_OUTER,
  VFX_LEVELUP_INNER,

  count,
};
struct SINGLE_DefaultParticles
{
  std::map<ParticleType, ParticleDescriptor> particles; // should match the above enum

  static SINGLE_DefaultParticles instance;
};

// the emitter that spawns the particle
struct ParticleEmitterComponent
{
  ParticleType particle_type;
  bool spawn_all_particles_at_once = false;
  bool expires = false;
  int particles_to_spawn_before_emitter_expires = 0;
  std::optional<engine::SRGBColour> colour = std::nullopt;
};

struct RequestToSpawnParticles
{
  glm::ivec2 position{ 0, 0 };
  ParticleType particle_type;

  float radius_pixels_lower = 0.0f;
  float radius_pixels_upper = 0.0f;
  entt::entity parent = entt::null;
  std::optional<engine::SRGBColour> colour = std::nullopt;
};

} // namespace game2d