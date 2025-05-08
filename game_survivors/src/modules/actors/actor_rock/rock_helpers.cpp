#include "pch.hpp"

#include "rock_helpers.hpp"

#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/maths/noise.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"

namespace game2d {

engine::SRGBColour
lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent)
{
  const auto a_lin = engine::SRGBToLinear(a);
  const auto b_lin = engine::SRGBToLinear(b);

  const float col_r = engine::lerp(a_lin.r, b_lin.r, percent);
  const float col_g = engine::lerp(a_lin.g, b_lin.g, percent);
  const float col_b = engine::lerp(a_lin.b, b_lin.b, percent);

  return engine::LinearToSRGB({ col_r, col_g, col_b, 1.0f });
};

struct NoiseInfo
{
  std::optional<float> noise = std::nullopt;
  glm::ivec2 xy = { 0, 0 };
};

// configs
const int wh = 50;
const float frequency = 0.1f;
const float tilesize = 25.0f;

std::vector<NoiseInfo>
generate_noise(entt::registry& r, float cutoff, float frequency, int seed)
{
  std::vector<NoiseInfo> generated;
  for (int y = 0; y < wh; y++) {
    for (int x = 0; x < wh; x++) {
      // convert from [-1, 1] to [0, 1]
      const auto raw_noise = perlin_noise_2d(x * frequency, y * frequency, seed);
      const auto noise = (raw_noise + 1.0f) * 0.5f;
      const auto grid_xy = glm::ivec2{ x, y };
      if (noise < cutoff) {
        // pad vector to maintain dimentions
        generated.push_back(NoiseInfo{ .noise = std::nullopt, .xy = grid_xy });
        continue;
      }
      generated.push_back(NoiseInfo{ .noise = noise, .xy = grid_xy });
    }
  }

  // clang-format off
   auto noise_it = generated 
    | std::views::filter([](const NoiseInfo& n) { return n.noise.has_value(); }) 
    | std::views::transform([](const NoiseInfo& n) { return n.noise.value(); });
  // clang-format on

  // clamp noise using the standard deviation to prevent outliers
  const std::vector<float> noise_vec = { noise_it.begin(), noise_it.end() };
  if (noise_vec.size() == 0)
    return {};
  const auto mean = std::accumulate(noise_vec.begin(), noise_vec.end(), 0.0f) / noise_vec.size();
  const auto sq_sum = std::transform_reduce(
    noise_vec.begin(), noise_vec.end(), 0.0f, std::plus<>(), [mean](float x) { return (x - mean) * (x - mean); });
  const auto stddev = std::sqrt(sq_sum / noise_vec.size());
  const float lower = mean - 2.0f * stddev;
  const float upper = mean + 2.0f * stddev;

  // clamp the noise.
  for (auto& g : generated) {
    if (g.noise.has_value())
      g.noise = glm::clamp(g.noise.value(), lower, upper);
  }

  return generated;
};

std::vector<std::vector<NoiseInfo>>
identify_islands(const std::vector<NoiseInfo>& generated, const float isovalue_threshold)
{
  // get the coordinates of any noise that is > threshold
  auto f = generated | std::views::filter([](const NoiseInfo& n) { return n.noise.has_value(); });

  std::vector<MapEntry> map_entries;
  for (int y = 0; y < wh; y++) {
    for (int x = 0; x < wh; x++) {
      const auto idx = engine::grid::grid_position_to_index({ x, y }, wh);
      const int cost = generated[idx].noise.has_value() ? 1.0f : -1.0f;
      map_entries.push_back(MapEntry{ .cost = cost });
    }
  }
  const Map_NonEntt map{ .xmax = wh, .ymax = wh, .map = map_entries };

  std::vector<NoiseInfo> unprocessed{ f.begin(), f.end() };
  std::vector<std::vector<NoiseInfo>> islands;

  while (unprocessed.size() > 0) {
    // pick any point with noise.
    const auto point = unprocessed[0];

    // do a floodfill algorithm from the noise point.
    std::vector<glm::ivec2> areas = generate_accessible_areas(map, { point.xy.x, point.xy.y }, wh * wh);

    // remove the found points from unprocessed.
    for (const auto& found : areas)
      std::erase_if(unprocessed, [&](const NoiseInfo& ni) { return ni.xy == found; });

    // add it as an island
    std::vector<NoiseInfo> island;
    for (const auto& found : areas)
      island.push_back(generated[engine::grid::grid_position_to_index({ found.x, found.y }, wh)]);
    islands.push_back(island);
  }

  SDL_Log("Found: %i islands", (int)islands.size());
  return islands;
};

//
// uses a marching square algorithm to generates contours for the islands, which we can feed in to box2d
//
std::vector<Edge>
generate_contours(entt::registry& r,
                  const std::vector<NoiseInfo>& island,
                  const float tilesize,
                  const float isovalue_threshold)
{
  enum class EDGES
  {
    B = 0,
    R = 1,
    T = 2,
    L = 3,
    ALL, // not currently used
  };

  // map the case to intersected edges
  const std::vector<std::vector<EDGES>> lookup_table = {
    {},                     // Case: 0,     0000
    { EDGES::L, EDGES::B }, // Case: 1      0001                bl
    { EDGES::B, EDGES::R }, // Case: 2      0010            br
    { EDGES::L, EDGES::R }, // Case: 3      0011            br  bl
    { EDGES::R, EDGES::T }, // Case: 4      0100        tr
    { EDGES::ALL },         // Case: 5      0101        tr      bl (ambiguous)
    { EDGES::T, EDGES::B }, // Case: 6      0110        tr  br
    { EDGES::T, EDGES::L }, // Case: 7      0111        tr  br  bl
    { EDGES::T, EDGES::L }, // Case: 8      1000    tl
    { EDGES::T, EDGES::B }, // Case: 9      1001    tl          bl
    { EDGES::ALL },         // Case: 10     1010    tl      br     (ambiguous)
    { EDGES::T, EDGES::R }, // Case: 11     1011    tl      br  bl
    { EDGES::L, EDGES::R }, // Case: 12     1100    tl  tr
    { EDGES::B, EDGES::R }, // Case: 13     1101    tl  tr      bl
    { EDGES::B, EDGES::L }, // Case: 14     1110    tl  tr  br
    {}                      // Case: 15     1111    tl  tr  br  bl
  };

  std::vector<Edge> island_contours;

  const auto get_noise = [&](int x, int y) -> float {
    auto it = std::find_if(island.begin(), island.end(), [&](const NoiseInfo& ni) { return ni.xy.x == x && ni.xy.y == y; });
    if (it == island.end())
      return 0.0f;
    return it->noise.value();
  };

  for (int y = 0; y < wh - 1; y++) {
    for (int x = 0; x < wh - 1; x++) {

      const float bl = get_noise(x, y);
      const float br = get_noise(x + 1, y);
      const float tr = get_noise(x + 1, y + 1);
      const float tl = get_noise(x, y + 1);

      // For the purposes of marching square,
      // anything above the isovalue gets identified as a "1"
      // anything below the isovalue gets identified as a "0"
      int state = 0;
      state |= (bl != 0.0f) ? 1 : 0;
      state |= (br != 0.0f) ? 2 : 0;
      state |= (tr != 0.0f) ? 4 : 0;
      state |= (tl != 0.0f) ? 8 : 0;

      const std::vector<EDGES> idxs = lookup_table[state];
      if (idxs.size() != 2)
        continue; // only interested in lines
      const auto e0 = idxs[0];
      const auto e1 = idxs[1];

      const auto xy = engine::grid::grid_space_to_world_space({ x, y }, tilesize);
      glm::vec2 p0 = xy;
      glm::vec2 p1 = xy;

      if (e0 == EDGES::L)
        p0.x -= 0.5f * tilesize;
      if (e0 == EDGES::R)
        p0.x += 0.5f * tilesize;
      if (e0 == EDGES::T)
        p0.y += 0.5f * tilesize;
      if (e0 == EDGES::B)
        p0.y -= 0.5f * tilesize;

      if (e1 == EDGES::L)
        p1.x -= 0.5f * tilesize;
      if (e1 == EDGES::R)
        p1.x += 0.5f * tilesize;
      if (e1 == EDGES::T)
        p1.y += 0.5f * tilesize;
      if (e1 == EDGES::B)
        p1.y -= 0.5f * tilesize;

      island_contours.push_back({ p0, p1 });
    }
  }

  return island_contours;
};

void
generate_rocks(entt::registry& r, const float cutoff)
{
  static int seed = 0;
#if defined(_DEBUG)
  seed++;
#else
  seed = engine::get_system_time_for_seed();
#endif

  // note: press kp 7 to regenerate
  SDL_Log("Generating rocks, cutoff: %f", cutoff);
  const auto generated = generate_noise(r, cutoff, frequency, seed);

  // whats the smallest & largest noise in the distribution
  auto filtered = generated | std::views::filter([](const NoiseInfo& n) { return n.noise.has_value(); });
  const auto min_compare = [](const NoiseInfo& a, const NoiseInfo& b) { return a.noise.value() < b.noise.value(); };
  const auto max_compare = [](const NoiseInfo& a, const NoiseInfo& b) { return a.noise.value() > b.noise.value(); };
  const auto min_it = std::min_element(filtered.begin(), filtered.end(), min_compare);
  const auto max_it = std::min_element(filtered.begin(), filtered.end(), max_compare);

  // identify the noise into islands.
  const auto islands = identify_islands(generated, 0.5f);

  // after converting in to islands, generate the contours (outline)
  for (int i = 0; const std::vector<NoiseInfo>& island : islands) {
    const auto contours = generate_contours(r, island, tilesize, cutoff);

    // offset so the grid doesnt start at (0, 0)
    const auto offset = (int)(-wh * 0.5f);
    const auto offset_worldspace = (int)offset * tilesize;
    std::vector<Edge> offset_contours = contours;
    for (auto& [p0, p1] : offset_contours) {
      p0.x += offset_worldspace;
      p0.y += offset_worldspace;
      p1.x += offset_worldspace;
      p1.y += offset_worldspace;
    }

    auto island_e = create_empty<RockComponent>(r);
    r.emplace<DebugContoursComponent>(island_e, offset_contours);

    /*
    for (const auto& info : island) {
      const auto xy = info.xy;

      //  remap [min_noise, max_noise] to [0, 1];
      const auto noise = engine::scale(info.noise.value(), min_it->noise.value(), max_it->noise.value(), 0.0f, 1.0f);

      // https://colorhunt.co/palette/a86523e9a319fad59afcefcb
      // https://colorhunt.co/palette/626f47a4b465f5ecd5f0bb78
      const auto sand_l = engine::SRGBColour{ 252, 238, 203, 255 };
      const auto sand_d = engine::SRGBColour{ 250, 213, 154, 255 };
      const auto grass_l = engine::SRGBColour{ 164, 180, 101, 255 };
      const auto grass_d = engine::SRGBColour{ 98, 111, 71, 255 };
      const float boundary_a = 0.1f;
      const float boundary_b = 0.2f;
      const float boundary_c = 1.0f;

      // engine::SRGBColour rock_col{ 1.0f, 1.0f, 1.0f, 1.0f };
      // if (noise < boundary_a) // light sand <=> dark sand
      //   rock_col = lerp_colour(sand_l, sand_d, (noise - 0.0f) / (boundary_a - 0.0f));
      // else if (noise < boundary_b)
      //   rock_col = lerp_colour(sand_d, grass_d, (noise - boundary_a) / (boundary_b - boundary_a));
      // else
      //   rock_col = lerp_colour(grass_d, grass_l, (noise - boundary_b) / (boundary_c - boundary_b));

      const auto tmp = (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f);
      const engine::SRGBColour rock_col{ tmp.x, tmp.y, tmp.z, tmp.w };

      const int offset = (int)(-wh * 0.5f);
      const auto worldspace = engine::grid::grid_space_to_world_space_center({ xy.x + offset, xy.y + offset }, tilesize);

      // const auto rock_e = spawn(r, "actor_enemy_rocks");
      // give_life(r, rock_e, worldspace, { tilesize, tilesize });
      // set_colour(r, rock_e, rock_col); // make the colour represent the noise value.
      // r.emplace<RockComponent>(rock_e);
      // r.emplace<TeamComponent>(rock_e, AvailableTeams::neutral);
      // r.get<DefaultColour>(rock_e).colour = rock_col;
      // r.remove<OnDeathCallbacks>(rock_e);
      // const auto rock_fixture_e = get_fixture_by_tag(r, rock_e, "fixture_core");
      // r.emplace<HealthComponent>(rock_fixture_e, HealthComponent{ 5000, 5000 });
    }
    */
    i++;
  }

  SDL_Log("Spawned: %i rocks", r.view<const RockComponent>().size());

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