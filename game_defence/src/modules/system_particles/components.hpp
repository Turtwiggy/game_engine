#pragma once

#include "colour/colour.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// related components:
// ScaleOverTimeComponent
// SpriteAnimationComponent
// EntityTimedLifecycle
// TransformComponent
// CooldownComponent

// used by non-physics body components
struct VelocityTemporaryComponent
{
  float x = 0;
  float y = 0;
};

struct ScaleOverTimeComponent
{
  float timer = 0.0f;
  float seconds_until_complete = 3.0f;
  float start_size = 16.0f;
  float end_size = 0.0f;
};

// each individual particle
struct ParticleDescription
{
  int time_to_live_ms = 1000;
  float start_size = 10;
  float end_size = 10;
  glm::ivec2 position{ 0, 0 };
  glm::vec2 velocity{ 0, 0 };
  std::string sprite = "EMPTY";
  engine::SRGBColour default_colour{ 1.0f, 1.0f, 1.0f, 1.0f };

  // int amount = 1;
  // bool slightly_change_velocity = false;

  // not implemented
  // engine::SRGBColour start_colour;
  // engine::SRGBColour end_colour;
};

// the emitter that spawns the particle
struct ParticleEmitterComponent
{
  ParticleDescription particle_to_emit;

  bool spawn_all_particles_at_once = false;

  bool expires = false;
  int particles_to_spawn_before_emitter_expires = 0;

  bool random_velocity = false;
};

struct RequestToSpawnParticles
{
  glm::ivec2 position{ 0, 0 };
};

struct SpawnParticlesOnDeath
{
  bool placeholder = true;
};

} // namespace game2d