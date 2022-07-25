#pragma once

#include "engine/colour.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace game2d {

// This is the loaded yml sprite information
struct SpriteAnimation
{
  std::string name;
  std::vector<glm::ivec2> animation_frames;
  float animation_frames_per_second = 1; // TODO: do something with this
  float animation_angle_degrees = 0.0f;
};

struct SpriteAnimationComponent
{
  std::string playing_animation_name;
  int frame = 0;
  float frame_dt = 0.0f;

  float speed = 1.0f;
  bool playing = true;
  bool looping = true;
};

struct SpriteComponent
{
  engine::LinearColour colour;
  int x = 0;
  int y = 0;
  int tex_unit = 0; // do not serialize
  float angle_radians = 0.0f;
};

struct SINGLETON_Animations
{
  std::vector<SpriteAnimation> animations;
};

} // namespace game2d