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
  float time_to_live_ms = 1000;
  float start_size = 10;
  float end_size = 10;
  glm::ivec2 position{ 0, 0 };
  glm::ivec2 velocity{ 0, 0 };
  std::string sprite = "EMPTY";
  engine::SRGBColour default_colour{ 1.0f, 1.0f, 1.0f, 1.0f };

  // not implemented
  engine::SRGBColour start_colour;
  engine::SRGBColour end_colour;
};

// the emitter that spawns the particle
struct ParticleEmitterComponent
{
  ParticleDescription particle_to_emit;
};

// TEMP: should not be here
struct SetPositionToParentsPosition
{
  glm::ivec2 offset{ 0, 0 };
};

} // namespace game2d