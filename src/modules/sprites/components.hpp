#pragma once

#include "engine/colour.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace game2d {

// Attach this to have the SpriteAnimationComponent
// automatically have the SpriteAnimation updated (walk cycles)
// based on velocity. There's scope to expand this.
struct AnimationSetByVelocityComponent
{
  bool placeholder = true;
};

// This is the loaded yml sprite information
struct SpriteAnimation
{
  std::string name;
  std::vector<glm::ivec2> animation_frames;
  float animation_frames_per_second = 1; // UNUSED
  float animation_angle_degrees = 0.0f;
};

// This contains the current state of the animation frame
struct SpriteAnimationComponent
{
  std::string playing_animation_name;
  int frame = 0;
  float frame_dt = 0.0f;
  float speed = 1.0f;
  bool playing = true;
  bool looping = true;
};

// Information required by the renderer
struct SpriteComponent
{
  engine::LinearColour colour;
  int x = 0;
  int y = 0;
  int tex_unit = 0; // do not serialize
  float angle_radians = 0.0f;

  // spritesheet info
  int sx = 0;
  int sy = 0;
};

// Contains sprite frame data from assets/config/spritemap_X.json
struct SINGLETON_Animations
{
  std::vector<SpriteAnimation> animations;
};

} // namespace game2d