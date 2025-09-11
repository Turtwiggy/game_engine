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
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_lighthouse/lighthouse_components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/renderer/lights/components.hpp"
#include "modules/systems/system_island_ai/island_ai_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"

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

  // clang-format off
   auto noise_it = generated 
    | std::views::filter([cutoff](const NoiseInfo& n) { return n.noise >= cutoff; }) 
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

  return generated;
};

std::vector<std::vector<NoiseInfo>>
identify_islands(const std::vector<NoiseInfo>& generated, const float isovalue_threshold)
{
  const auto wh = SINGLE_Islands::instance.wh;

  // get the coordinates of any noise that is > threshold
  auto f =
    generated | std::views::filter([isovalue_threshold](const NoiseInfo& n) { return n.noise >= isovalue_threshold; });

  std::vector<MapEntry> map_entries;
  for (int y = 0; y < wh; y++) {
    for (int x = 0; x < wh; x++) {
      const auto idx = engine::grid::grid_position_to_index({ x, y }, wh);
      const int cost = generated[idx].noise >= isovalue_threshold ? 1.0f : -1.0f;
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
                  const std::vector<NoiseInfo>& island,
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

  const auto get_noise = [&](int x, int y) -> float {
    auto it = std::find_if(island.begin(), island.end(), [&](const NoiseInfo& ni) { return ni.xy.x == x && ni.xy.y == y; });
    auto noise = (it == island.end()) ? 0.0f : it->noise;
    return noise >= isovalue_threshold ? noise : 0.0f;
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

void
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
  b2ChainId chain_id = b2CreateChain(body_c.bodyId, &chain_def);

  // PhysicsFixtureComponent fixture_c;
  // fixture_c.bodyId = bodyId;
  // fixture_c.shapeId = shape_id;
  // auto fixture_e = create_empty<PhysicsFixtureComponent>(r, fixture_c);
  // r.emplace<HasParentComponent>(fixture_e, island_e);
  // b2Body_SetUserData(fixture_c.bodyId, (void*)fixture_e); // box2d: give link to entt
  // auto& child_c = r.get_or_emplace<HasChildrenComponent>(island_e);
  // child_c.children.push_back(fixture_e);
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

  glBindTexture(GL_TEXTURE_2D, ri.tex_id_heightmap.id);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_FLOAT, data.data());
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
  const auto frequency = SINGLE_Islands::instance.frequency;
  const auto tilesize = SINGLE_Islands::instance.tilesize;
  const auto wh = SINGLE_Islands::instance.wh;

  {
    auto generated = generate_noise(r, cutoff, frequency, seed);

    // Modify the noise, so that the center is always an island.
    for (int x = 22; x < 29; x++) {
      for (int y = 22; y < 29; y++) {
        const auto at_grid_xy = [&](NoiseInfo& info) { return info.xy == glm::ivec2{ x, y }; };
        auto it = std::find_if(generated.begin(), generated.end(), at_grid_xy);
        if (it == generated.end())
          continue;
        it->noise = 0.8; // make it solid
      }
    }

    SINGLE_Islands::instance.generated = std::move(generated);
  }

  const auto& generated = SINGLE_Islands::instance.generated;

  // identify the noise into islands.
  const auto islands = identify_islands(generated, cutoff);

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
    create_box2d_shape(r, island_e, offset_contours);
    generate_rock_bounding_box(r, island_e);

    i++;
  }

  SDL_Log("Spawned: %i rocks", r.view<const RockComponent>().size());

  // Upload the heightmap data to the gpu.
  SDL_Log("Uploading heightmap data to gpu");
  upload_heightmap_to_gpu(r);
};

engine::SRGBColour
get_colour_of_tile(entt::registry& r,
                   const glm::vec2 xy,
                   const glm::ivec2 unoffset_xy,
                   const DebugContoursComponent& contours_c,
                   const float min_noise,
                   const float max_noise)
{
  auto rock_col = engine::SRGBColour{ 1.0f, 0.0f, 0.0f, 1.0f };
  const auto& info = contours_c.island_noise;
  const auto at_xy = [&unoffset_xy](const NoiseInfo& noise) { return noise.xy == unoffset_xy; };
  const auto it = std::find_if(info.begin(), info.end(), at_xy); // the noise info should always exist
  if (it == info.end()) {
    //
    // noise info for island missing; something aint right
    //
    const auto debug_e = spawn(r, "empty");
    give_life(r, debug_e, { xy.x, xy.y }, { 5, 5 });
    set_colour(r, debug_e, rock_col);
    return rock_col;
  }
  const auto ni = (*it);

  //  remap [min_noise, max_noise] to [0, 1];
  const auto noise = engine::scale(ni.noise, min_noise, max_noise, 0.0f, 1.0f);

  // https://colorhunt.co/palette/a86523e9a319fad59afcefcb
  // https://colorhunt.co/palette/626f47a4b465f5ecd5f0bb78
  const auto sand_l = engine::SRGBColour{ 252, 238, 203, 255 };
  const auto sand_d = engine::SRGBColour{ 250, 213, 154, 255 };
  const auto grass_l = engine::SRGBColour{ 164, 180, 101, 255 };
  const auto grass_d = engine::SRGBColour{ 98, 111, 71, 255 };

  // boundary (noise) between [0, 1] where we change the colours
  const float boundary_a = 0.1f;
  const float boundary_b = 0.2f;
  const float boundary_c = 1.0f;

  if (noise < boundary_a) // light sand <=> dark sand
    rock_col = lerp_colour(sand_l, sand_d, (noise - 0.0f) / (boundary_a - 0.0f));
  else if (noise < boundary_b)
    rock_col = lerp_colour(sand_d, grass_d, (noise - boundary_a) / (boundary_b - boundary_a));
  else
    rock_col = lerp_colour(grass_d, grass_l, (noise - boundary_b) / (boundary_c - boundary_b));

  return rock_col;
}

void
generate_island_interior(entt::registry& r)
{
  auto& islands_c = SINGLE_Islands::instance;
  const auto& generated = islands_c.generated;
  const auto tilesize = islands_c.tilesize;
  const float half_tilesize = tilesize * 0.5f;

  // whats the smallest & largest noise in the distribution
  // auto filtered =
  //   generated | std::views::filter([isovalue_threshold](const NoiseInfo& n) { return n.noise >= isovalue_threshold; });
  // const auto min_compare = [](const NoiseInfo& a, const NoiseInfo& b) { return a.noise < b.noise; };
  // const auto max_compare = [](const NoiseInfo& a, const NoiseInfo& b) { return a.noise > b.noise; };
  // const auto min_it = std::min_element(filtered.begin(), filtered.end(), min_compare);
  // const auto max_it = std::min_element(filtered.begin(), filtered.end(), max_compare);

  // given each island...
  for (const auto [island_e, island_c, bb_c, contours_c] :
       r.view<const RockComponent, const BoundingBoxComponent, DebugContoursComponent>().each()) {
    const auto center = 0.5f * (bb_c.br + bb_c.tl);

    const auto tl = bb_c.tl;
    const auto wh = bb_c.br - bb_c.tl;
    const auto tl_gridpos = engine::grid::worldspace_to_gridspace(bb_c.tl, tilesize);

    const float min_x = bb_c.tl.x + half_tilesize;
    const float min_y = bb_c.tl.y + half_tilesize;
    const float max_x = bb_c.tl.x + wh.x - half_tilesize;
    const float max_y = bb_c.tl.y + wh.y - half_tilesize;
    for (float x = min_x; x <= max_x; x += tilesize) {
      for (float y = min_y; y <= max_y; y += tilesize) {

        const float tol = 2.5;

        if (!crossing_number_algorithm__point_is_inside({ x, y }, contours_c.sorted_edges)) {
          // const auto debug_e = spawn(r, "empty");
          // give_life(r, debug_e, { x, y }, { 5, 5 });
          // set_colour(r, debug_e, { 1.0f, 0.0f, 0.0f, 1.0f });
          continue;
        }

        const auto pos = glm::vec2{ x, y };
        const auto xy = engine::grid::worldspace_to_gridspace(pos, tilesize);
        contours_c.all_island_xy.push_back(xy);

        // note: we offset the islands by (-0.5 * wh)
        // this is not reflected in the island_noise (which contains xy grid coords)
        // when searching, undo this offset
        const int map_wh = islands_c.wh;
        const auto offset = (int)(-map_wh * 0.5f);
        const auto unoffset_xy = xy - offset + glm::ivec2{ 1, 1 };

        // work out colour of tile given noise.
        // const auto rock_col = get_colour_of_tile(r, pos, unoffset_xy, contours_c, min_it->noise, max_it->noise);
        const auto rock_col = engine::SRGBColour{ 255, 60, 60, 255 };

        const auto debug_e = spawn(r, "empty");
        r.get<TagComponent>(debug_e).tag = "empty-IslandSquare";
        give_life(r, debug_e, pos, { 3, 3 });
        // give_life(r, debug_e, pos, { tilesize, tilesize });
        set_colour(r, debug_e, rock_col); // make the colour represent the noise value.
        set_z_index(r, debug_e, ZLayer::FLOOR);

        const auto id = engine::encode_cantor_pairing_function(xy.x, xy.y);
        islands_c.id_to_island_eid.emplace(id, island_e);

        // the island_gridspace is relative to the tl of the island
        // const auto island_gridspace = tl_gridpos - gridpos;
      }
    }
  }
}

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
  const auto center_worldspace = glm::vec2{ 0, 0 }; // base island hould always have a tile at 0, 0
  const auto center_gridspace = engine::grid::worldspace_to_gridspace(center_worldspace, tilesize);
  const auto center_id = engine::encode_cantor_pairing_function(center_gridspace.x, center_gridspace.y);
  const auto center_eid = SINGLE_Islands::instance.id_to_island_eid.at(center_id);
  assert(center_id != entt::null);
  return center_eid;
};

void
spawn_lighthouse(entt::registry& r, DebugContoursComponent& island_c, const glm::ivec2 gridpos)
{
  const auto tilesize = SINGLE_Islands::instance.tilesize;

  const auto thing_e = spawn(r, "actor_lighthouse");
  auto pos = engine::grid::gridspace_to_worldspace_center(gridpos, tilesize);
  pos += glm::vec2{ tilesize * 0.5f, tilesize * 0.5f }; // off grid
  give_life(r, thing_e, pos, { tilesize, tilesize });

  // todo: set random rotation and slightly varying speed
  r.emplace<LighthouseComponent>(thing_e);
  r.emplace<LightEmitterComponent>(thing_e);
  // r.emplace<LightTypeWedge>(thing_e);
  r.emplace<LightTypeCircle>(thing_e);
  // auto popup_e = create_popup(r, center, "Lighthouse");
  // r.remove<EntityTimedLifecycle>(popup_e);
  // r.get<WiggleUpAndDown>(popup_e).amplitude = 1.0f;

  island_c.occupied_island_xy.push_back({ gridpos, thing_e });
};

entt::entity
spawn_islander(entt::registry& r,
               engine::RandomState& rnd,
               const entt::entity island_e,
               std::string tag,
               const AvailableTeams team,
               const bool has_brain = false)
{

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  auto& island_c = r.get<DebugContoursComponent>(island_e);

  const auto unoccupied = get_unoccupied_tiles(island_c);
  const auto xy = unoccupied[(int)engine::rand_det_s(rnd.rng, 0, (int)unoccupied.size())];
  const auto thing_e = spawn(r, tag);
  auto pos = engine::grid::gridspace_to_worldspace(xy, tilesize);
  pos += glm::vec2{ tilesize, tilesize }; // off grid
  give_life(r, thing_e, pos, { tilesize, tilesize });
  r.emplace<IslandDwellerComponent>(thing_e);
  r.emplace<HealthComponent>(thing_e, HealthComponent{ 3, 3 });
  r.emplace<TeamComponent>(thing_e, TeamComponent{ .team = team });

  // let the thing move
  // add brains to enemies
  r.emplace<MovementIslandComponent>(thing_e, MovementIslandComponent{ .island_e = island_e });
  r.emplace<IslanderAiComponent>(thing_e);

  island_c.occupied_island_xy.push_back({ xy, thing_e });
  return thing_e;
};

#if defined(_DEBUG)
static engine::RandomState spawn_rnd(0); // same roll every time
#else
static engine::RandomState spawn_rnd(engine::get_system_time_for_seed());
#endif

void
generate_island_life__base_island(entt::registry& r)
{
  const auto tilesize = SINGLE_Islands::instance.tilesize;

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

  const auto idx_0 = engine::rand_det_s(spawn_rnd.rng, (int)0, (int)animal_keys.size());
  const auto idx_1 = engine::rand_det_s(spawn_rnd.rng, (int)0, (int)animal_keys.size());
  spawn_islander(r, spawn_rnd, center_island_eid, animal_keys[idx_0], AvailableTeams::player, true);
  spawn_islander(r, spawn_rnd, center_island_eid, animal_keys[idx_1], AvailableTeams::player, true);
}

void
generate_island_life__other_islands(entt::registry& r)
{
  const auto center_island_eid = get_center_island_eid(r);

  // spawn things on the islands

  const auto tilesize = SINGLE_Islands::instance.tilesize;
  for (const auto& [e, island_c, bb_c] : r.view<DebugContoursComponent, const BoundingBoxComponent>().each()) {

    if (e == center_island_eid)
      continue; // dont spawn mobs on the base island

    // TODO: generate a spawn rate table for enemies.
    spawn_islander(r, spawn_rnd, e, "actor_islanddweller_pirate", AvailableTeams::enemy, true);
    spawn_islander(r, spawn_rnd, e, "actor_islanddweller_spider", AvailableTeams::enemy, true);
    spawn_islander(r, spawn_rnd, e, "actor_islanddweller_scorpion", AvailableTeams::enemy, true);
  }
};

void
set_players_as_landed(entt::registry& r)
{
  // forcefully land all boats to start.
  for (int i = 0; const auto& [e, player_c] : r.view<const PlayerBoatComponent>().each()) {

    auto base_island_e = get_center_island_eid(r);
    auto& base_island_c = r.get<DebugContoursComponent>(base_island_e);
    const auto& bb_c = r.get<BoundingBoxComponent>(base_island_e);
    const auto unoccupied_tiles = get_unoccupied_tiles(base_island_c);
    const auto tilesize = SINGLE_Islands::instance.tilesize;
    const std::vector<glm::vec2> player_dir{ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } }; // t, r, b, l

    const auto center_worldspace_tl = glm::vec2{ 0, 0 } - glm::vec2{ tilesize * 0.5, tilesize * 0.5 };
    const auto offset = glm::vec2{ tilesize * 0.5, tilesize * 0.5 } * player_dir[i];
    const auto center_worldspace_adj = center_worldspace_tl + offset;
    const auto center_gridspace = engine::grid::worldspace_to_gridspace(center_worldspace_adj, tilesize);
    land_player_on_island(r, base_island_c, center_gridspace, e, base_island_e);

    i++;
  }
};

glm::vec2
get_player_spawn_point_around_starting_island(entt::registry& r, int idx)
{
  // const auto pos = rnd_position_in_map_but_not_inside_players_or_islands(r);
  const auto base_island_eid = get_center_island_eid(r);
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
  const float offset = 32.0f; // Distance from edge towards outside
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