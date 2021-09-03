#pragma once

// c++ lib header
#include <tuple>
#include <vector>

// engine headers
#include "engine/opengl/shader.hpp"

// game headers
#include "2d_game_object.hpp"

namespace game2d {

struct PointLight
{
  GameObject2D light_object;
  // distance 200
  // const float light_linear = 0.022f;
  // const float light_quadratic = 0.0019f;
  // distance 325
  // const float light_linear = 0.014f;
  // const float light_quadratic = 0.0007f;
  // distance 600
  // const float light_linear = 0.007f;
  // const float light_quadratic = 0.0002f;
  // distance 3250
  // const float light_linear = 0.0014f;
  // const float light_quadratic =  0.000007f;
  float linear = 0.0014f;
  float quadratic = 0.000007f;
  bool cast_shadows = false;
};

struct SceneicPointLight : public PointLight
{
  SceneicPointLight()
  {
    this->linear = 0.0014f;
    this->quadratic = 0.000007f;
    this->cast_shadows = true;
  };
};

struct MuzzleFlashPointLight : public PointLight
{
  MuzzleFlashPointLight()
  {
    this->linear = 0.0014f;
    this->quadratic = 0.0019f;
  }
};

void
generate_intersections(GameObject2D& camera,
                       glm::ivec2& light_pos,
                       const std::vector<std::reference_wrapper<GameObject2D>>& ents,
                       const glm::ivec2& screen_wh,
                       std::vector<std::tuple<float, float, float>>& intersections);

} // namespace game2d
