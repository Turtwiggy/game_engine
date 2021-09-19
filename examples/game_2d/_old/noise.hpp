
#pragma once

// engine headers
#include <engine/maths_core.hpp>

// other project headers
#include <glm/glm.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

inline std::vector<float>
generate_noise(engine::RandomState& rng, glm::ivec2 size)
{
  std::vector<float> ret;
  ret.resize(size.x * size.y);

  for (auto& i : ret) {
    i = engine::rand_det_s(rng.rng, 0, 1);
  }

  return ret;
}

inline float
positive_fmod(float x, float y)
{
  float result = std::remainder(std::fabs(x), (y = std::fabs(y)));

  if (std::signbit(result)) {
    result += y;
  }

  return result;
}

struct noise_data
{
  std::vector<float> noise_1;
  std::vector<float> noise_2;
  std::vector<float> noise_3;
  std::vector<float> noise_4;
};

inline float
simple_sample(const std::vector<float>& data, glm::vec2 pos, glm::ivec2 size)
{
  glm::vec2 tl = glm::floor(pos);
  glm::vec2 br = glm::ceil(pos);

  float xfrac = (pos.x - tl.x);
  float yfrac = (pos.y - tl.y);

  tl.x = positive_fmod(tl.x, size.x - 1);
  tl.y = positive_fmod(tl.y, size.y - 1);

  br.x = positive_fmod(br.x, size.x - 1);
  br.y = positive_fmod(br.y, size.y - 1);

  float tl_val = data[((int)tl.y) * size.x + (int)tl.x];
  float tr_val = data[((int)tl.y) * size.x + (int)br.x];
  float bl_val = data[((int)br.y) * size.x + (int)tl.x];
  float br_val = data[((int)br.y) * size.x + (int)br.x];

  float y1 = glm::mix(tl_val, tr_val, xfrac);
  float y2 = glm::mix(bl_val, br_val, xfrac);

  return glm::mix(y1, y2, yfrac);
}

inline float
sample(noise_data nd, glm::vec2 pos, glm::ivec2 size)
{
  float sample_freq = 0.005;

  glm::vec2 warp = { simple_sample(nd.noise_2, pos * sample_freq, size),
                     simple_sample(nd.noise_3, pos * sample_freq, size) };
  glm::vec2 warp2 = glm::vec2{ simple_sample(nd.noise_2, pos * sample_freq * 10.0f, size),
                               simple_sample(nd.noise_3, pos * sample_freq * 10.0f, size) };

  glm::vec2 wpos = pos + warp * 40.0f + warp2 * 20.0f;

  float density = 0;

  density += simple_sample(nd.noise_1, wpos, size);
  density += simple_sample(nd.noise_1, wpos / 2.f, size) * 2;
  density += simple_sample(nd.noise_1, wpos / 4.f, size) * 4;
  density += simple_sample(nd.noise_1, wpos / 8.f, size) * 8;

  float width = size.x;

  float water_width = width * 0.8;
  float land_width = width - water_width;

  glm::vec2 centre = glm::vec2{ size.x, size.y } / 2.f;

  float distance_from_centre = (pos - centre).length();

  distance_from_centre = glm::clamp(distance_from_centre, 0.f, width / 2);

  float final_density = density / (8 + 4 + 2 + 1);

  if (distance_from_centre >= land_width / 2) {
    float water_frac = (distance_from_centre - (land_width / 2)) / (water_width / 2);

    float subtractive_density = simple_sample(nd.noise_4, wpos / 32.f, size);

    float low_val = final_density - subtractive_density;

    low_val = glm::clamp(low_val, 0.f, 1.f);

    return glm::mix(final_density, low_val, water_frac);
  }

  return final_density;
}

} // namespace game2d