#pragma once

namespace game2d {

struct PointLight
{
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
    this->quadratic = 0.07f;
  }
};

} // namespace game2d