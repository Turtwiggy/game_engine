#include "pch.hpp"

#include "rock_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/collision.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/maths/noise.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/actors/actor_island_drum/drum_component.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/actors/actor_islander/islander_helpers.hpp"
#include "modules/actors/actor_lighthouse/lighthouse_components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/actors/actor_rock/rock_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/renderer/lights/components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_island_ai/island_ai_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"
#include "modules/systems/system_shoot_auto/autofire_components.hpp"
#include "modules/systems/system_shoot_auto/autofire_helpers.hpp"
#include "modules/ui/ui_island_interact_system/ui_island_interact_components.hpp"
#include "resources/data.hpp"


namespace game2d {

bool
is_island(const float noise, const float cutoff)
{
  return noise >= cutoff;
};

// engine::SRGBColour
// lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent)
// {
//   const auto a_lin = engine::SRGBToLinear(a);
//   const auto b_lin = engine::SRGBToLinear(b);
//   const float col_r = engine::lerp(a_lin.r, b_lin.r, percent);
//   const float col_g = engine::lerp(a_lin.g, b_lin.g, percent);
//   const float col_b = engine::lerp(a_lin.b, b_lin.b, percent);
//   return engine::LinearToSRGB({ col_r, col_g, col_b, 1.0f });
// };

std::vector<NoiseInfo>
generate_noise(entt::registry& r, float cutoff, float frequency, int seed)
{
  const auto wh = SINGLE_Islands::instance.wh;

  std::vector<NoiseInfo> generated;
  for (int y = 0; y < wh; y++) {
    for (int x = 0; x < wh; x++) {
      // convert from [-1, 1] to [0, 1]
      const auto raw_noise = perlin_noise_2d(x * frequency, y * frequency, seed);
      const auto noise = (raw_noise + 1.0f) * 0.5f;
      const auto grid_xy = glm::ivec2{ x, y };
      generated.push_back(NoiseInfo{ .noise = noise, .xy = grid_xy });
    }
  }

  // std::string n_str = "";
  // for (int x = 0; x < wh; x++) {
  //   for (int y = 0; y < wh; y++) {
  //     n_str += std::format("{:0.2f} ", generated[x * wh + y].noise).c_str();
  //   }
  //   n_str += "\n";
  // }
  // SDL_Log("noise generated: \n %s", n_str.c_str());

  /*

  // clang-format off
   auto noise_it = generated
    | std::views::filter([cutoff](const NoiseInfo& n) { return n.noise > cutoff; })
    | std::views::transform([](const NoiseInfo& n) { return n.noise; });
  // clang-format on

  // clamp noise using the standard deviation to prevent outliers
  const std::vector<float> noise_vec = { noise_it.begin(), noise_it.end() };
  if (noise_vec.empty())
    return {};
  const auto mean = std::accumulate(noise_vec.begin(), noise_vec.end(), 0.0f) / noise_vec.size();
  const auto sq_sum = std::transform_reduce(
    noise_vec.begin(), noise_vec.end(), 0.0f, std::plus<>(), [mean](float x) { return (x - mean) * (x - mean); });
  const auto stddev = std::sqrt(sq_sum / noise_vec.size());
  const float lower = mean - 2.0f * stddev;
  const float upper = mean + 2.0f * stddev;

  // clamp the noise.
  for (auto& g : generated)
    g.noise = glm::clamp(g.noise, lower, upper);

  */

  return generated;
};

// keep generating noise until the "core" gameplay square contains noise
std::vector<NoiseInfo>
generate_noise__with_base_island(entt::registry& r)
{
  static int seed = 0;
  const auto frequency = SINGLE_Islands::instance.frequency;
  const auto cutoff = SINGLE_Islands::instance.cutoff;
  // SINGLE_Islands::instance.valid = false;
  SDL_Log("Generate noise: frequency: %f, cutoff: %f", frequency, cutoff);

#if defined(_DEBUG)
  seed++;
#else
  seed = engine::get_system_time_for_seed();
#endif

  std::vector<NoiseInfo> generated_final;

  {
    seed++;
    auto generated = generate_noise(r, cutoff, frequency, seed);

    // get the coordinates of any noise that is > threshold
    // auto f = generated | std::views::filter([cutoff](const NoiseInfo& n) { return is_island(n.noise, cutoff); });
    // SDL_Log("noise above cutoff %i", (int)std::distance(f.begin(), f.end()));

    // check the noise so that the center is always an island.
    // note: we also adjust it, that the borders of the island is clear for a couple tiles.
    //
    const auto island_center = (int)(0.5 * SINGLE_Islands::instance.wh);
    const auto min_x = island_center - 2;
    const auto max_x = island_center + 2;
    const auto min_x_boundary = min_x - 6;
    const auto max_x_boundary = max_x + 6;

    for (int y = min_x_boundary; y < max_x_boundary; y++) {
      for (int x = min_x_boundary; x < max_x_boundary; x++) {
        const auto idx = engine::grid::grid_position_to_index({ x, y }, SINGLE_Islands::instance.wh);
        if (y >= min_x && y <= max_x && x >= min_x && x <= max_x)
          generated[idx].noise = cutoff;
        else
          generated[idx].noise = 0.0f;
      }
    }

    // for (int y = min_x; y < max_x; y++) {
    //   for (int x = min_x; x < max_x; x++) {
    //     const auto idx = engine::grid::grid_position_to_index({ x, y }, SINGLE_Islands::instance.wh);
    //     generated[idx].noise = cutoff;
    //   }
    // }

    generated_final = std::move(generated);
  }

  return generated_final;
};

std::vector<std::vector<NoiseInfo>>
identify_islands(const std::vector<NoiseInfo>& generated, const float isovalue_threshold)
{
  const auto wh = SINGLE_Islands::instance.wh;
  const auto tilesize = (float)SINGLE_Islands::instance.tilesize;

  // get the coordinates of any noise that is > threshold
  auto f = generated |
           std::views::filter([isovalue_threshold](const NoiseInfo& n) { return is_island(n.noise, isovalue_threshold); });
  SDL_Log("noise above cutoff %i", (int)std::distance(f.begin(), f.end()));

  std::vector<MapEntry> map_entries;
  for (int y = 0; y < wh; y++) {
    for (int x = 0; x < wh; x++) {
      const auto idx = engine::grid::grid_position_to_index({ x, y }, wh);
      const auto land = is_island(generated[idx].noise, isovalue_threshold);
      const int cost = land ? 1 : -1;
      map_entries.push_back(MapEntry{ .cost = cost });
    }
  }
  const Map_NonEntt map{ .xmax = wh, .ymax = wh, .map = map_entries };

  std::vector<NoiseInfo> unprocessed{ f.begin(), f.end() };
  std::vector<std::vector<NoiseInfo>> islands;

  while (!unprocessed.empty()) {
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

    //
    // validate island(s)
    //

    // ignore islands that are too small.
    const auto min_island_size = 4;
    if (island.size() < min_island_size)
      continue; // discard island

    // ignore islands that border the edge.
    const auto borders_map_edge = [wh](const NoiseInfo& ni) {
      return ni.xy.x == 0 || ni.xy.x == wh - 1 || ni.xy.y == 0 || ni.xy.y == wh - 1;
    };
    auto it = std::find_if(island.begin(), island.end(), borders_map_edge);
    if (it != island.end())
      continue; // discard island,

    islands.push_back(island);
  }

  SDL_Log("Found: %i islands", (int)islands.size());
  return islands;
};

//
// uses a marching square algorithm to generates contours for the islands, which we can feed in to box2d
//

struct ContoursOut
{
  std::vector<Edge> contours;
  std::vector<Edge> sorted_contours;
};

std::vector<Edge>
sort_contours(const std::vector<Edge>& island_contours)
{
  std::vector<Edge> sorted_island_contours;
  sorted_island_contours.push_back(island_contours[0]);

  for (size_t i = 1; i < island_contours.size(); i++) {

    const auto prv = sorted_island_contours[i - 1];
    bool added = false;

    for (int j = 0; j < island_contours.size(); j++) {
      auto cur = island_contours[j];
      if (prv == cur)
        continue;

      if (prv.b == cur.a) {
        added = true;
        sorted_island_contours.push_back(cur);
        break;
      }

      if (prv.b == cur.b) {
        added = true;
        // the edges are the wrong way round
        std::swap(cur.a, cur.b);
        sorted_island_contours.push_back(cur);
        break;
      }
    }

    if (!added)
      break; // no valid connected contour foound.
  }
  return sorted_island_contours;
};

ContoursOut
generate_contours(entt::registry& r,
                  const std::vector<NoiseInfo>& island_noise,
                  const float tilesize,
                  const float isovalue_threshold)
{
  const auto wh = SINGLE_Islands::instance.wh;

  enum class EDGES
  {
    L = 1,
    R = 2,
    T = 3,
    B = 4,
    ALL, // not currently used
  };

  // map the case to intersected edges
  const std::vector<std::vector<EDGES>> lookup_table = {
    {},                     // Case: 0,     0000
    { EDGES::B, EDGES::L }, // Case: 1      0001                bl
    { EDGES::B, EDGES::R }, // Case: 2      0010            br
    { EDGES::R, EDGES::L }, // Case: 3      0011            br  bl
    { EDGES::R, EDGES::T }, // Case: 4      0100        tr
    { EDGES::ALL },         // Case: 5      0101        tr      bl (ambiguous)
    { EDGES::B, EDGES::T }, // Case: 6      0110        tr  br
    { EDGES::T, EDGES::L }, // Case: 7      0111        tr  br  bl
    { EDGES::T, EDGES::L }, // Case: 8      1000    tl
    { EDGES::T, EDGES::B }, // Case: 9      1001    tl          bl
    { EDGES::ALL },         // Case: 10     1010    tl      br     (ambiguous)
    { EDGES::T, EDGES::R }, // Case: 11     1011    tl      br  bl
    { EDGES::R, EDGES::L }, // Case: 12     1100    tl  tr
    { EDGES::B, EDGES::R }, // Case: 13     1101    tl  tr      bl
    { EDGES::B, EDGES::L }, // Case: 14     1110    tl  tr  br
    {}                      // Case: 15     1111    tl  tr  br  bl
  };

  std::vector<Edge> island_contours;

  // note: we search the island's noise entries, not all generated noise.
  const auto get_noise = [&](int x, int y) -> float {
    auto it = std::find_if(
      island_noise.begin(), island_noise.end(), [&](const NoiseInfo& ni) { return ni.xy.x == x && ni.xy.y == y; });

    if (it == island_noise.end())
      return 0.0f;

    return is_island(it->noise, isovalue_threshold) ? 1.0f : 0.0f;
  };

  for (int y = 0; y < wh - 1; y++) {
    for (int x = 0; x < wh - 1; x++) {

      const float tl = get_noise(x, y + 1);
      const float tr = get_noise(x + 1, y + 1);
      const float br = get_noise(x + 1, y);
      const float bl = get_noise(x, y);

      // For the purposes of marching square,
      // anything above the isovalue gets identified as a "1"
      // anything below the isovalue gets identified as a "0"
      int state = 0;
      state |= (tl != 0.0f) ? 8 : 0;
      state |= (tr != 0.0f) ? 4 : 0;
      state |= (br != 0.0f) ? 2 : 0;
      state |= (bl != 0.0f) ? 1 : 0;

      const std::vector<EDGES> idxs = lookup_table[state];
      if (idxs.size() != 2)
        continue; // only interested in lines

      const auto e0 = idxs[0];
      const auto e1 = idxs[1];

      const auto xy = engine::grid::gridspace_to_worldspace({ x, y }, tilesize);
      glm::vec2 p0 = xy;
      glm::vec2 p1 = xy;

      const glm::vec2 dl{ 0.0f, 0.5f };
      const glm::vec2 dr{ 1.0f, 0.5f };
      const glm::vec2 dt{ 0.5f, 1.0f };
      const glm::vec2 db{ 0.5f, 0.0f };

      if (e0 == EDGES::L)
        p0 += tilesize * dl;
      if (e0 == EDGES::R)
        p0 += tilesize * dr;
      if (e0 == EDGES::T)
        p0 += tilesize * dt;
      if (e0 == EDGES::B)
        p0 += tilesize * db;

      if (e1 == EDGES::L)
        p1 += tilesize * dl;
      if (e1 == EDGES::R)
        p1 += tilesize * dr;
      if (e1 == EDGES::T)
        p1 += tilesize * dt;
      if (e1 == EDGES::B)
        p1 += tilesize * db;

      island_contours.push_back({ p0, p1 });
    }
  }

  ContoursOut out;
  out.contours = island_contours;
  out.sorted_contours = sort_contours(island_contours);
  return out;
};

entt::entity
create_box2d_shape(entt::registry& r, entt::entity island_e, const std::vector<Edge>& contours)
{
  auto& physics_c = get_first_component<SINGLE_Physics>(r);

  b2BodyDef def = b2DefaultBodyDef();
  def.type = b2_staticBody;
  def.userData = (void*)static_cast<uintptr_t>(entt::to_integral(island_e));
  const auto bodyId = b2CreateBody(physics_c.worldId, &def);
  const auto& body_c = r.emplace<PhysicsBodyComponent>(island_e, PhysicsBodyComponent{ .bodyId = bodyId });

  // Create Fixture(s)
  std::vector<glm::ivec2> contour_pixels;
  for (int i = 0; i < contours.size(); i++) {
    if (i > 0) {
      const auto hmm_b = glm::ivec2(contours[i - 1].b.x, contours[i - 1].b.y);
      const auto hmm_a = glm::ivec2(contours[i - 0].a.x, contours[i - 0].a.y);
      assert(hmm_a == hmm_b);
    }
    contour_pixels.push_back(glm::ivec2{ contours[i].a.x, contours[i].a.y });
  }

  std::vector<b2Vec2> contor_meters;
  std::transform(contour_pixels.begin(), contour_pixels.end(), std::back_inserter(contor_meters), [](const glm::vec2& d) {
    return b2Vec2{ pixels_to_meters(d.x), pixels_to_meters(d.y) };
  });

  b2ChainDef chain_def = b2DefaultChainDef();
  chain_def.points = contor_meters.data();
  chain_def.count = (int)contor_meters.size();
  chain_def.isLoop = true;
  chain_def.enableSensorEvents = true;
  // chain_def.userData =
  b2ChainId chain_id = b2CreateChain(body_c.bodyId, &chain_def);

  PhysicsFixtureComponent fixture_c;
  fixture_c.bodyId = bodyId;
  fixture_c.chainId = chain_id;
  auto fixture_e = create_empty<PhysicsFixtureComponent>(r, fixture_c);
  r.emplace_or_replace<TagComponent>(fixture_e, TagComponent{ "fixture_rock" });
  r.emplace<HasParentComponent>(fixture_e, island_e);

  // set userData on each segment
  int segments = b2Chain_GetSegmentCount(chain_id);
  std::vector<b2ShapeId> segs(segments);
  b2Chain_GetSegments(chain_id, segs.data(), segments);
  for (const b2ShapeId seg : segs)
    b2Shape_SetUserData(seg, (void*)fixture_e); // box2d: give link to entt

  auto& child_c = r.get_or_emplace<HasChildrenComponent>(island_e);
  child_c.children.push_back(fixture_e);

  return fixture_e;
};

void
generate_rock_bounding_box(entt::registry& r, entt::entity e)
{
  const auto& contours_c = r.get<DebugContoursComponent>(e);

  BoundingBoxComponent bb_c;

  for (const auto& points : contours_c.sorted_edges) {
    bb_c.tl.x = glm::min(bb_c.tl.x, (float)points.a.x);
    bb_c.tl.y = glm::min(bb_c.tl.y, (float)points.a.y);
    bb_c.br.x = glm::max(bb_c.br.x, (float)points.a.x);
    bb_c.br.y = glm::max(bb_c.br.y, (float)points.a.y);
  }

  r.emplace<BoundingBoxComponent>(e, bb_c);
};

/*
void
upload_heightmap_to_gpu(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const int width = ri.heightmap_texture_wh;
  const int height = ri.heightmap_texture_wh;
  std::vector<float> data(width * height, 0.0f);

  // load heightmap data in to vector.
  const auto& generated = SINGLE_Islands::instance.generated;

  for (const auto& ni : generated) {
    const auto tex_idx = engine::grid::grid_position_to_index({ ni.xy.x, ni.xy.y }, width);
    data[tex_idx] = ni.noise;
  }

// Find out some stats about the heightmap.
#if defined(_DEBUG)
  const auto& f = generated;
  const auto min_compare = [](const NoiseInfo& a, const NoiseInfo& b) { return a.noise < b.noise; };
  const auto min_it = std::min_element(f.begin(), f.end(), min_compare);
  const auto max_it = std::max_element(f.begin(), f.end(), min_compare);
  SDL_Log("(uploading) Min noise: %f, Max noise: %f", min_it->noise, max_it->noise);
#endif

  glBindTexture(GL_TEXTURE_2D, ri.tex_id_heightmap.id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_FLOAT, data.data());
};
*/

void
generate_position_id_to_island_eid_map(entt::registry& r)
{
  auto& islands_c = SINGLE_Islands::instance;
  const auto& generated = islands_c.generated;
  const auto tilesize = islands_c.tilesize;
  const float half_tilesize = tilesize * 0.5f;

  // given each island...
  for (const auto [island_e, island_c, bb_c, contours_c] :
       r.view<const RockComponent, const BoundingBoxComponent, DebugContoursComponent>().each()) {
    const auto center = 0.5f * (bb_c.br + bb_c.tl);

#if defined(_DEBUG)
    int before_count = islands_c.id_to_island_eid.size();
#endif

    const auto tl = bb_c.tl;
    const auto wh = bb_c.br - bb_c.tl;
    const auto tl_gridpos = engine::grid::worldspace_to_gridspace(bb_c.tl, tilesize);
    const auto offset = glm::vec2{ half_tilesize, half_tilesize };

    const float min_x = bb_c.tl.x;
    const float min_y = bb_c.tl.y;
    const float max_x = bb_c.tl.x + wh.x - tilesize;
    const float max_y = bb_c.tl.y + wh.y - tilesize;
    for (float x = min_x; x <= max_x; x += tilesize) {
      for (float y = min_y; y <= max_y; y += tilesize) {

        const auto pos = glm::vec2{ x, y };
        const auto xy = engine::grid::worldspace_to_gridspace(pos, tilesize);
        const auto pos2 = engine::grid::gridspace_to_worldspace_center(xy, tilesize) + offset;

        const auto& c = contours_c.all_island_xy;
        auto it = std::find(c.begin(), c.end(), xy);
        if (it != c.end())
          continue; // xy already existed

        const auto xy_adj = glm::vec2{ x, y } + offset;
        if (!crossing_number_algorithm__point_is_inside(xy_adj, contours_c.sorted_edges)) {
#if defined(_DEBUG)
          // const auto debug_e = spawn(r, "empty");
          // give_life(r, debug_e, pos2, { 4, 4 });
          // set_colour(r, debug_e, { 1.0f, 0.0f, 0.0f, 1.0f });
          // set_z_index(r, debug_e, ZLayer::FOREGROUND);
#endif
          continue;
        }

        contours_c.all_island_xy.push_back(xy);

        const auto id = engine::encode_cantor_pairing_function(xy.x, xy.y);

        if (islands_c.id_to_island_eid.contains(id)) {
          // oops! one id shouldnt contain multiple islands.
          throw std::runtime_error("One id cant contain multiple islands");
        }

        islands_c.id_to_island_eid.emplace(id, island_e);

        // the island_gridspace is relative to the tl of the island
        // const auto island_gridspace = tl_gridpos - gridpos;
      }
    }

#if defined(_DEBUG)
    // check check that all the island tiles are added to the island_to_id_eid
    int after_count = islands_c.id_to_island_eid.size();
    assert(before_count + contours_c.all_island_xy.size() == after_count);
#endif
  }
};

void
generate_rocks(entt::registry& r)
{
  // note: press kp 7 to regenerate
  const auto frequency = SINGLE_Islands::instance.frequency;
  const auto cutoff = SINGLE_Islands::instance.cutoff;
  const auto tilesize = SINGLE_Islands::instance.tilesize;
  const auto wh = SINGLE_Islands::instance.wh;
  SDL_Log("Generating rocks, cutoff: %f", cutoff);

  int cur_islands = 0;
  int min_islands = 7;
  std::vector<std::vector<NoiseInfo>> islands;

  while (cur_islands < min_islands) {
    SINGLE_Islands::instance.generated = generate_noise__with_base_island(r);
    const auto& generated = SINGLE_Islands::instance.generated;
    const std::vector<std::vector<NoiseInfo>> i = identify_islands(generated, cutoff);
    cur_islands = (int)i.size();
    islands = i;
    SDL_Log("Iteration... found %i islands", cur_islands);
  }

  // after converting in to islands, generate the contours (outline)
  for (int i = 0; const std::vector<NoiseInfo>& island : islands) {
    const auto contours = generate_contours(r, island, tilesize, cutoff);

    // offset so the grid doesnt start at (0, 0)
    const auto offset = (int)(-wh * 0.5f);
    const auto offset_worldspace = (int)offset * tilesize;
    std::vector<Edge> offset_contours = contours.sorted_contours;
    for (auto& [p0, p1] : offset_contours) {
      p0.x += offset_worldspace;
      p0.y += offset_worldspace;
      p1.x += offset_worldspace;
      p1.y += offset_worldspace;
    }

    auto island_e = create_empty<RockComponent>(r);

    DebugContoursComponent debug_c;
    debug_c.edges = contours.contours;
    debug_c.sorted_edges = offset_contours;
    debug_c.island_noise = island;
    r.emplace<DebugContoursComponent>(island_e, debug_c);
    r.emplace<TeamComponent>(island_e, TeamComponent{ AvailableTeams::neutral });

    // island contours in to box2d to create collisions
    auto fixture_e = create_box2d_shape(r, island_e, offset_contours);
    generate_rock_bounding_box(r, island_e);

    r.emplace<IslandFixtureComponent>(fixture_e);

    i++;
  }

  SDL_Log("Spawned: %i rocks", (int)r.view<const RockComponent>().size());

  // Upload the heightmap data to the gpu.
  SDL_Log("Uploading heightmap data to gpu");
  // upload_heightmap_to_gpu(r);

  // populate id_to_island_eid
  generate_position_id_to_island_eid_map(r);
};

// entt::entity
// get_island_eid(entt::registry& r, glm::vec2 pos)
// {
//   //
// }

entt::entity
get_center_island_eid(entt::registry& r)
{
  const auto& island_ids_to_eid = SINGLE_Islands::instance.id_to_island_eid;

  // Extract all unique entt::entity values into a vector
  // std::unordered_set<entt::entity> unique_entities;
  // for (const auto& [id, eid] : island_ids_to_eid)
  //   unique_entities.emplace(eid);

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  const auto wh = SINGLE_Islands::instance.wh;
  const auto center_worldspace = glm::vec2{ 0, 0 }; // base island should always have a tile at 0, 0
  const auto center_gridspace = engine::grid::worldspace_to_gridspace(center_worldspace, tilesize);
  const auto center_id = engine::encode_cantor_pairing_function(center_gridspace.x, center_gridspace.y);
  const auto center_eid = SINGLE_Islands::instance.id_to_island_eid.at(center_id);
  assert(center_id != entt::null);
  return center_eid;
};

void
spawn_lighthouse(entt::registry& r, DebugContoursComponent& island_c, const glm::ivec2 gridpos)
{
  const auto tilesize_unit = default_map_unit_tilesize;
  const auto tilesize_map = SINGLE_Islands::instance.tilesize;

  const auto thing_e = spawn(r, "actor_lighthouse");
  auto pos = engine::grid::gridspace_to_worldspace_center(gridpos, tilesize_map);
  pos += glm::vec2{ tilesize_map * 0.5f, tilesize_map * 0.5f }; // off grid
  give_life(r, thing_e, pos, { tilesize_unit, tilesize_unit });

  // todo: set random rotation and slightly varying speed
  r.emplace<LighthouseComponent>(thing_e);
  r.emplace<LightEmitterComponent>(thing_e);
  // r.emplace<LightTypeWedge>(thing_e);
  r.emplace<LightTypeCircle>(thing_e);
  // auto popup_e = create_popup(r, center, "Lighthouse");
  // r.remove<EntityTimedLifecycle>(popup_e);
  // r.get<WiggleUpAndDown>(popup_e).amplitude = 1.0f;

  // tmp
  // r.emplace<InteractableComponent>(thing_e);

  island_c.occupied_island_xy.push_back({ gridpos, thing_e });
};

void
spawn_cannon(entt::registry& r, engine::RandomState& spawn_rnd, entt::entity island_eid)
{
  auto cannon_e = spawn_islander_unoccupied_edge(r, spawn_rnd, island_eid, "actor_island_cannon", AvailableTeams::player);
  r.remove<HealthComponent>(cannon_e);
  r.remove<SpriteComponent>(cannon_e);

  const auto weapon_data = get_weapon_data(r, "weapon_island_cannon");
  const auto weapon_e = spawn_weapon(r, weapon_data, "weapon_island_cannon", { default_map_tilesize, default_map_tilesize });
  r.emplace<WeaponDef>(weapon_e, get_weapon_def(r, weapon_e));
  r.emplace<BulletDef>(weapon_e, get_bullet_def(r, weapon_e));
  r.emplace<AutofireComponent>(weapon_e);
  r.emplace<IslandCannonComponent>(weapon_e);
  connect_parent_and_weapon(r, cannon_e, weapon_e);
  set_sprite(r, weapon_e, "CROSSBOW_37_5");
};

#if defined(_DEBUG)
static engine::RandomState spawn_rnd(0); // same roll every time
#else
static engine::RandomState spawn_rnd(engine::get_system_time_for_seed());
#endif

void
generate_island_life__base_island(entt::registry& r)
{
  auto tilesize = default_map_unit_tilesize;

  // on the base island
  const auto center_island_eid = get_center_island_eid(r);
  auto& island_c = r.get<DebugContoursComponent>(center_island_eid);
  auto& bb_c = r.get<BoundingBoxComponent>(center_island_eid);

  // give the base island a lighthouse
  // const auto center_worldspace = 0.5f * (bb_c.br + bb_c.tl);
  // const auto center_gridspace = engine::grid::worldspace_to_gridspace(center_worldspace, tilesize);
  const auto island_center_gridspace = glm::ivec2{ -1, -1 }; // the island is not symmetrical
  spawn_lighthouse(r, island_c, island_center_gridspace);

  const auto animal_keys = std::vector<std::string>{
    "actor_islanddweller_common_person", //
    "actor_islanddweller_animal_duck_0", //
    "actor_islanddweller_animal_duck_1", //
    "actor_islanddweller_animal_cow",    //
    "actor_islanddweller_animal_horse",  //
    "actor_islanddweller_animal_pig",    //
    "actor_islanddweller_animal_cat",    //
    "actor_islanddweller_animal_dog",    //
  };

  // const auto idx_0 = engine::rand_det_s(spawn_rnd.rng, (int)0, (int)animal_keys.size());
  // spawn_islander_unoccupied(r, spawn_rnd, center_island_eid, animal_keys[idx_0], AvailableTeams::player, true);
  // const auto idx_1 = engine::rand_det_s(spawn_rnd.rng, (int)0, (int)animal_keys.size());
  // spawn_islander_unoccupied(r, spawn_rnd, center_island_eid, animal_keys[idx_1], AvailableTeams::player, true);

  // spawn a drum
  // auto drum_e =
  //   spawn_islander_unoccupied_edge(r, spawn_rnd, center_island_eid, "actor_island_item_drum", AvailableTeams::player);
  // r.emplace<DrumComponent>(drum_e);
  // r.remove<HealthComponent>(drum_e);

  // spawn a broken cannon
  spawn_cannon(r, spawn_rnd, center_island_eid);
}

void
generate_island_life__other_islands(entt::registry& r)
{
  const auto center_island_eid = get_center_island_eid(r);

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  for (const auto& [e, island_c, bb_c] : r.view<DebugContoursComponent, const BoundingBoxComponent>().each()) {

    if (e == center_island_eid)
      continue; // dont spawn mobs on the base island

    const auto enemy_keys = std::vector<std::string>{
      "actor_islanddweller_pirate",   //
      "actor_islanddweller_spider",   //
      "actor_islanddweller_scorpion", //
    };

    // TODO: generate a spawn rate table for enemies.
    int island_squares = (int)island_c.all_island_xy.size();

    // e.g. 10 => 1 enemy
    // e.g. 20 => 5 enemies
    // e.g. 30 == 7 enemies
    int enemies_to_spawn = glm::floor(island_squares * 0.1f);
    enemies_to_spawn = glm::clamp(enemies_to_spawn, 1, 10);
    for (int i = 0; i < enemies_to_spawn; i++) {
      const auto idx = engine::rand_det_s(spawn_rnd.rng, (int)0, (int)enemy_keys.size());
      spawn_islander_unoccupied(r, spawn_rnd, e, enemy_keys[idx], AvailableTeams::enemy, true);
    }

    // spawn_islander_unoccupied(r, spawn_rnd, e, "actor_islanddweller_pirate", AvailableTeams::enemy, true);
    // spawn_islander_unoccupied(r, spawn_rnd, e, "actor_islanddweller_spider", AvailableTeams::enemy, true);
    // spawn_islander_unoccupied(r, spawn_rnd, e, "actor_islanddweller_scorpion", AvailableTeams::enemy, true);
  }
};

void
set_players_as_landed(entt::registry& r)
{
  const auto tilesize_map = SINGLE_Islands::instance.tilesize;
  const auto tilesize_units = default_map_unit_tilesize;

  // forcefully land all boats to start.
  for (int i = 0; const auto& [e, player_c] : r.view<const PlayerBoatComponent>().each()) {

    auto base_island_e = get_center_island_eid(r);
    auto& base_island_c = r.get<DebugContoursComponent>(base_island_e);
    const auto& bb_c = r.get<BoundingBoxComponent>(base_island_e);
    const auto unoccupied_tiles = get_unoccupied_tiles(base_island_c);
    const std::vector<glm::vec2> player_dir{ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }; // t, r, b, l

    const auto center_worldspace_tl = glm::vec2{ 0, 0 } - glm::vec2{ tilesize_map * 0.5, tilesize_map * 0.5 };
    const auto offset = glm::vec2{ tilesize_map * 0.5, tilesize_map * 0.5 } * player_dir[i];
    const auto center_worldspace_adj = center_worldspace_tl + offset;
    const auto center_gridspace = engine::grid::worldspace_to_gridspace(center_worldspace_adj, tilesize_map);
    land_player_on_island(r, base_island_c, center_gridspace, e, base_island_e);

    i++;
  }
};

glm::vec2
get_player_spawn_point_around_starting_island(entt::registry& r, int idx)
{
  // const auto pos = rnd_position_in_map_but_not_inside_players_or_islands(r);
  const auto base_island_eid = get_center_island_eid(r);

  // todo: work out why DebugContoursComponent didnt exist on island_eid

  const auto& base_island_c = r.get<const DebugContoursComponent>(base_island_eid);
  const auto& base_island_aabb = r.get<const BoundingBoxComponent>(base_island_eid);

  // given a bounding box with .tl and .br
  // and given a player index [0, 1, 2, 3],
  // spawn players on the top center, right center, bottom center, and left center edges of the bounding box
  // Determine spawn position for each player based on index
  // clang-format off
  glm::vec2 pos{0, 0};
  const float center_x = (base_island_aabb.tl.x + base_island_aabb.br.x) * 0.5f;
  const float center_y = (base_island_aabb.tl.y + base_island_aabb.br.y) * 0.5f;
  const float offset = default_map_tilesize; // Distance from edge towards outside
  switch (idx) {
    case 0: pos = { center_x, base_island_aabb.tl.y - offset }; break; // t
    case 1: pos = { base_island_aabb.br.x + offset, center_y }; break; // r
    case 2: pos = { center_x, base_island_aabb.br.y + offset }; break; // b
    case 3: pos = { base_island_aabb.tl.x - offset, center_y }; break; // l
  }
  // clang-format on

  return pos;
};

} // namespace game2d