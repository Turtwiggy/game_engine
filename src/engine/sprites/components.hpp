#pragma once

#include "engine/colour/colour.hpp"

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <utility>
#include <vector>

namespace game2d {

struct Spritesheet
{
  std::string name;
  std::string path;

  // texture size in pixels
  int px_total = 0;
  int py_total = 0;

  // number of pixels per sprite
  int px = 32;
  int py = 32;

  // number of sprites
  int nx = 1;
  int ny = 1;
};

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
  // key: spritesheet path. value: vector of loaded animations
  std::vector<std::pair<Spritesheet, std::vector<SpriteAnimation>>> animations;

  static SINGLE_Animations instance;
};

struct SpriteTriangleComponent
{
  glm::vec2 a{ 0, 0 };
  glm::vec2 b{ 0, 0 };
  glm::vec2 c{ 0, 0 };
  engine::SRGBColour a_colour = engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f);
  engine::SRGBColour b_colour = engine::SRGBColour(0.0f, 1.0f, 0.0f, 1.0f);
  engine::SRGBColour c_colour = engine::SRGBColour(0.0f, 0.0f, 1.0f, 1.0f);
  glm::vec2 uv_0 = { 0, 0 };
  glm::vec2 uv_1 = { 1, 1 };
  glm::vec2 uv_2 = { 1, 1 };
};

// Used by renderer
struct SpriteComponent
{
  engine::SRGBColour colour = engine::SRGBColour(1.0f, 1.0f, 1.0f, 1.0f);
  float angle_radians = 0.0f;

  SpritePosition tex_pos;
  int tex_unit = 0; // do not serialize

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
struct SpriteAnimationState
{
  std::string playing_animation_name;
  float timer = 0.0f;
  float duration = 10.0f; // seconds
  // float speed = 1.0f;
  bool playing = true;
  bool looping = true;
  // bool destroy_after_play = true;
};

} // namespace game2d