#pragma once

#include "colour/colour.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace game2d {

struct SpritePosition
{
  int x = 0;
  int y = 0;
  int w = 1;
  int h = 1;
};

// This is the loaded yml sprite information
struct SpriteAnimation
{
  std::string name;
  std::vector<SpritePosition> animation_frames;
  float angle_degrees = 0;
  // float animation_frames_per_second = 1;
};

// Contains sprite frame data from assets/config/spritemap_X.json
struct SINGLE_Animations
{
  std::vector<SpriteAnimation> animations;
};

struct SpriteComponent
{
  engine::LinearColour colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 1.0f, 1.0f, 1.0f));
  float angle_radians = 0.0f;

  SpritePosition tex_pos;
  int tex_unit = 0; // do not serialize

  // spritesheet info
  int total_sx = 0;
  int total_sy = 0;
  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(SpriteComponent, x, y, angle_radians, sx, sy);
};

// Attach this to have the SpriteAnimationComponent
// automatically have the SpriteAnimation updated (walk cycles)
// based on velocity. There's scope to expand this.
// struct AnimationSetByVelocityComponent
// {
//   bool placeholder = true;
// };

// This contains the current state of the animation
struct SpriteAnimationComponent
{
  std::string playing_animation_name;
  float timer = 0.0f;
  float duration = 10.0f;
  // float speed = 1.0f;
  bool playing = true;
  bool looping = true;
};

} // namespace game2d