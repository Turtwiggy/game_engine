#include "pch.hpp"

#include "rock_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/maths/noise.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/procedural/poisson.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/raws/raws_components.hpp"

namespace game2d {

engine::SRGBColour
lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent)
{
  const auto a_lin = engine::SRGBToLinear(a);
  const auto b_lin = engine::SRGBToLinear(b);

  // if (a_lin.r > b_lin.r)
  //   std::swap(a_lin.r, b_lin.r);
  // if (a_lin.g > b_lin.g)
  //   std::swap(a_lin.g, b_lin.g);
  // if (a_lin.b > b_lin.b)
  //   std::swap(a_lin.b, b_lin.b);

  const float col_r = engine::lerp(a_lin.r, b_lin.r, percent);
  const float col_g = engine::lerp(a_lin.g, b_lin.g, percent);
  const float col_b = engine::lerp(a_lin.b, b_lin.b, percent);

  return engine::LinearToSRGB({ col_r, col_g, col_b, 1.0f });
}

struct NoiseInfo
{
  float noise = 0.0f;
  glm::ivec2 xy = { 0, 0 };
};

void
generate_rocks(entt::registry& r, const float cutoff)
{
  float frequency = 0.1f;
  std::vector<NoiseInfo> generated;

  static int seed = 0;
#if defined(_DEBUG)
  seed++;
#else
  seed = engine::get_system_time_for_seed();
#endif

  for (int y = -50; y < 50; y++) {
    for (int x = -50; x < 50; x++) {
      // convert from [-1, 1] to [0, 1]
      const auto raw_noise = perlin_noise_2d(x * frequency, y * frequency, seed);
      const auto noise = (raw_noise + 1.0f) * 0.5f;
      const auto grid_xy = glm::ivec2{ x, y };
      // SDL_Log("%i %i %f gp: %i %i", x, y, noise, grid_xy.x, grid_xy.y);
      if (noise < cutoff)
        continue;
      generated.push_back(NoiseInfo{ .noise = noise, .xy = grid_xy });
    }
  }

  // clamp noise using the standard deviation to prevent outliers
  const auto noise_it = generated | std::views::transform([](const auto& n) { return n.noise; });
  const auto noise_vec = std::vector<float>{ noise_it.begin(), noise_it.end() };
  if (noise_vec.size() == 0)
    return;
  const auto mean = std::accumulate(noise_vec.begin(), noise_vec.end(), 0.0f) / noise_vec.size();
  const auto sq_sum = std::transform_reduce(
    noise_vec.begin(), noise_vec.end(), 0.0f, std::plus<>(), [mean](float x) { return (x - mean) * (x - mean); });
  const auto stddev = std::sqrt(sq_sum / noise_vec.size());
  const float lower = mean - 2.0f * stddev;
  const float upper = mean + 2.0f * stddev;
  for (auto& g : generated)
    g.noise = glm::clamp(g.noise, lower, upper);

  // whats the smallest & largest noise in the distribution
  const auto min_compare = [](const auto& a, const auto& b) { return a.noise < b.noise; };
  const auto max_compare = [](const auto& a, const auto& b) { return a.noise > b.noise; };
  const auto min_it = std::min_element(generated.begin(), generated.end(), min_compare);
  const auto max_it = std::min_element(generated.begin(), generated.end(), max_compare);

  // after generating & messing with noise, generate rocks
  for (const auto& info : generated) {
    const auto xy = info.xy;

    // remap [min_noise, max_noise] to [0, 1];
    const auto noise = engine::scale(info.noise, min_it->noise, max_it->noise, 0.0f, 1.0f);

    // https://colorhunt.co/palette/a86523e9a319fad59afcefcb
    // https://colorhunt.co/palette/626f47a4b465f5ecd5f0bb78
    const auto sand_l = engine::SRGBColour{ 252, 238, 203, 255 };
    const auto sand_d = engine::SRGBColour{ 250, 213, 154, 255 };
    const auto grass_l = engine::SRGBColour{ 164, 180, 101, 255 };
    const auto grass_d = engine::SRGBColour{ 98, 111, 71, 255 };
    const float boundary_a = 0.1f;
    const float boundary_b = 0.2f;
    const float boundary_c = 1.0f;

    engine::SRGBColour rock_col{ 1.0f, 1.0f, 1.0f, 1.0f };
    if (noise < boundary_a) // light sand <=> dark sand
      rock_col = lerp_colour(sand_l, sand_d, (noise - 0.0f) / (boundary_a - 0.0f));
    else if (noise < boundary_b)
      rock_col = lerp_colour(sand_d, grass_d, (noise - boundary_a) / (boundary_b - boundary_a));
    else
      rock_col = lerp_colour(grass_d, grass_l, (noise - boundary_b) / (boundary_c - boundary_b));

    const auto worldspace = engine::grid::grid_space_to_world_space_center(xy, 10);
    const auto rock_e = spawn(r, "actor_enemy_rocks");
    give_life(r, rock_e, worldspace, { 10, 10 });
    set_colour(r, rock_e, rock_col); // make the colour represent the noise value.
    r.emplace<RockComponent>(rock_e);
    r.emplace<TeamComponent>(rock_e, AvailableTeams::neutral);
    r.get<DefaultColour>(rock_e).colour = rock_col;

    const auto rock_fixture_e = get_fixture_by_tag(r, rock_e, "fixture_core");
    r.emplace<HealthComponent>(rock_fixture_e, HealthComponent{ 5000, 5000 });
  }

  /*
  // e.g. wh 2000x2000 / rad (250*250) spawns <64 rocks
  PoissonIn in;
  in.seed = 0;
  in.radius = 400;
  in.wh = { 2000, 2000 };
  in.tl = { -in.wh.x * 0.5f, -in.wh.y * 0.5f };
  const auto out = generate_poisson(in);
  SDL_Log("Spawned %i poisson points for rocks", (int)out.results.size());
  for (const auto& point : out.results) {
  // spawn rock cluster
  auto rock_e = spawn(r, "actor_enemy_rocks");
  give_life(r, rock_e, point, { 256, 256 });
  r.emplace<TeamComponent>(rock_e, TeamComponent{ AvailableTeams::neutral });

  // rotate the rocks
  static engine::RandomState rock_rnd(0);
  r.get<TransformComponent>(rock_e).rotation_radians.z = engine::rand_det_s(rock_rnd.rng, 0.0f, engine::TWO_PI);
  }
  */
}

} // namespace game2d