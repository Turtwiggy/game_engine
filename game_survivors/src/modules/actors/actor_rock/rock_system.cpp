#include "pch.hpp"

#include "rock_components.hpp"
#include "rock_helpers.hpp"
#include "rock_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "engine/std/unordered_set/glm_hash.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_above_fog/above_fog_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

struct IslandEdgeLine
{
  Edge edge;
  float timer = 0.0f;
  const float edge_size_base = 4;
};

glm::vec2
get_perp(glm::vec2 ea, glm::vec2 eb)
{
  const auto dir = eb - ea;
  const auto pos_perp = engine::normalize_safe(glm::vec2{ -dir.y, dir.x });
  const auto neg_perp = engine::normalize_safe(-1.0f * pos_perp);
  return neg_perp;
};

void
create_island_triangles(entt::registry& r)
{
  // spawn the island sprite (verts => triangles)
  // the problem with this method is that the center doesnt see all the edges if the shape is concave
  // for (const auto edge : debug_c.sorted_edges) {
  //   const auto spawned_e = spawn(r, "empty");
  //   give_life(r, spawned_e, { 0, 0 }, { 0, 0 });
  //   r.remove<SpriteComponent>(spawned_e);
  //   SpriteTriangleComponent spr = {
  //     .a = center,
  //     .b = edge.a,
  //     .c = edge.b,
  //     .a_colour = engine::SRGBToLinear({ 0, 0, 0, 255 }),
  //     .b_colour = engine::SRGBToLinear({ 35, 100, 96, 255 }),
  //     .c_colour = engine::SRGBToLinear({ 35, 100, 96, 255 }),
  //   };
  //   r.emplace<SpriteTriangleComponent>(spawned_e, spr);
  // }

  for (const auto& [contours_e, debug_c, bb_c] : r.view<const DebugContoursComponent, const BoundingBoxComponent>().each()) {

    // get island center.
    const float center_x = (bb_c.tl.x + bb_c.br.x) * 0.5f;
    const float center_y = (bb_c.tl.y + bb_c.br.y) * 0.5f;
    const glm::vec2 center{ center_x, center_y };

    // x0, y0, x1, y1, ...
    std::unordered_set<glm::vec2, vec2_hash> set;
    for (const auto edge : debug_c.sorted_edges) {
      set.insert(edge.a);
      set.insert(edge.b);
    }
    std::vector<glm::vec2> verts(set.begin(), set.end());

    // verts <=> cdt conversion.
    std::vector<CDT::V2d<float>> coords;
    for (const auto vert : verts)
      coords.push_back({ vert.x, vert.y });

    // verts <=> edges conversion
    std::vector<CDT::Edge> edges;
    for (int i = 0; const auto edge : debug_c.sorted_edges) {
      // note: takes in vertex indicies.
      const auto comp_a = [&](const auto& a) { return a.x == edge.a.x && a.y == edge.a.y; };
      const auto comp_b = [&](const auto& a) { return a.x == edge.b.x && a.y == edge.b.y; };
      const auto a_it = std::find_if(verts.begin(), verts.end(), comp_a);
      const auto a_idx = static_cast<int>(a_it - verts.begin());
      const auto b_it = std::find_if(verts.begin(), verts.end(), comp_b);
      const auto b_idx = static_cast<int>(b_it - verts.begin());
      edges.push_back({ CDT::VertInd(a_idx), CDT::VertInd(b_idx) });
    }

    CDT::Triangulation<float> cdt;
    cdt.insertVertices(coords);
    cdt.insertEdges(edges);
    cdt.eraseOuterTrianglesAndHoles();

    auto& children_c = r.get_or_emplace<HasChildrenComponent>(contours_e);
    for (auto tri : cdt.triangles) {
      const auto v0 = cdt.vertices[tri.vertices[0]];
      const auto v1 = cdt.vertices[tri.vertices[1]];
      const auto v2 = cdt.vertices[tri.vertices[2]];
      const auto a = glm::vec2{ v0.x, v0.y };
      const auto b = glm::vec2{ v1.x, v1.y };
      const auto c = glm::vec2{ v2.x, v2.y };

      const auto spawned_e = spawn(r, "empty");
      give_life(r, spawned_e, { 0, 0 }, { 0, 0 });
      r.remove<SpriteComponent>(spawned_e);
      SpriteTriangleComponent spr = {
        .a = a,
        .b = b,
        .c = c,
        .a_colour = engine::SRGBToLinear({ 100, 100, 100, 255 }),
        .b_colour = engine::SRGBToLinear({ 100, 100, 100, 255 }),
        .c_colour = engine::SRGBToLinear({ 100, 100, 100, 255 }),
      };
      r.emplace<SpriteTriangleComponent>(spawned_e, spr);
      r.emplace<HasParentComponent>(spawned_e, contours_e);
      children_c.children.push_back(spawned_e);

      auto& tag_c = r.get<TagComponent>(spawned_e);
      tag_c.tag = "tri_island";
    }
  }
}

void
create_shore_triangles(entt::registry& r)
{
  // spawn some more edges, but offset them by the normal.
  const float thickness = 16;

  for (const auto& [contours_e, debug_c, bb_c] : r.view<const DebugContoursComponent, const BoundingBoxComponent>().each()) {

#if defined(_DEBUG)
    // static engine::RandomState angle_rnd(0); // same roll every time
    static engine::RandomState wave_rnd(engine::get_system_time_for_seed());
#else
    static engine::RandomState wave_rnd(engine::get_system_time_for_seed());
#endif

    // randomly offset the timer so all the islands look slightly different.
    const float timer_offset = engine::rand_det_s(wave_rnd.rng, 0.0f, 100.0f);
    for (int i = 0; i < debug_c.sorted_edges.size(); i++) {

      const auto& prv_edge = debug_c.sorted_edges[engine::wrap(i - 1, debug_c.sorted_edges.size())];
      const auto& cur_edge = debug_c.sorted_edges[i];
      const auto& nxt_edge = debug_c.sorted_edges[engine::wrap(i + 1, debug_c.sorted_edges.size())];

      const auto ea = glm::vec2(cur_edge.a);
      const auto eb = glm::vec2(cur_edge.b);
      const auto edge_midpoint = 0.5f * (ea + eb);

      // give each island a different colour
      // const auto tmp = (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f);
      // const engine::SRGBColour col{ tmp.x, tmp.y, tmp.z, tmp.w };
      // // const engine::SRGBColour col{ 1.0f, 1.0f, 1.0f, 1.0f };
      // {
      //   const auto line_info = generate_line(ea, eb, 2.0f);
      //   const auto spawned_e = spawn(r, "empty");
      //   give_life(r, spawned_e, { 0, 0 }, { 0, 0 });
      //   set_position_and_size_with_line(r, spawned_e, line_info);
      //   set_colour(r, spawned_e, col);
      //   r.emplace<IslandEdgeLine>(spawned_e, IslandEdgeLine{ .edge = cur_edge, .timer = timer_offset });
      // }

      const auto a = ea;
      const auto b = eb;
      const auto perp = get_perp(ea, eb);

      // adjust the outside points based to go to the midpoint of this point and the prv-edge outside point.
      const auto prv_edge_perp = get_perp(prv_edge.a, prv_edge.b);
      const auto prv_d = glm::vec2(prv_edge.b) + prv_edge_perp * thickness;
      const auto cur_c = ea + perp * thickness;
      const auto adj_c = 0.5f * (prv_d + cur_c);
      const auto c = adj_c;
      // const auto c = ea + neg_perp * 50.0f;

      const auto nxt_edge_perp = get_perp(nxt_edge.a, nxt_edge.b);
      const auto nxt_c = glm::vec2(nxt_edge.a) + nxt_edge_perp * thickness;
      const auto cur_d = eb + perp * thickness;
      const auto adj_d = 0.5f * (nxt_c + cur_d);
      const auto d = adj_d;
      // const auto d = eb + neg_perp * 50.0f;

      /*
      {
        const auto line_info2 = generate_line(ea, c, 2);
        const auto spawned_e2 = spawn(r, "empty");
        give_life(r, spawned_e2, { 0, 0 }, { 0, 0 });
        set_position_and_size_with_line(r, spawned_e2, line_info2);
        set_colour(r, spawned_e2, { 0.0f, 1.0f, 0.0f, 1.0f });
        r.emplace<IslandEdgeLine>(spawned_e2, IslandEdgeLine{ .edge = cur_edge, .timer = timer_offset });
      }
      {
        const auto line_info2 = generate_line(eb, d, 2);
        const auto spawned_e2 = spawn(r, "empty");
        give_life(r, spawned_e2, { 0, 0 }, { 0, 0 });
        set_position_and_size_with_line(r, spawned_e2, line_info2);
        set_colour(r, spawned_e2, { 0.0f, 0.0f, 1.0f, 1.0f });
        r.emplace<IslandEdgeLine>(spawned_e2, IslandEdgeLine{ .edge = cur_edge, .timer = timer_offset });
      }
      */

      // create shore as a quad (two triangles)
      {
        SpriteTriangleComponent tri1 = {
          .a = a,
          .b = b,
          .c = c,
          .a_colour = engine::SRGBToLinear({ 0.8f, 0.0f, 0.0f, 1.0f }),
          .b_colour = engine::SRGBToLinear({ 0.1f, 1.0f, 0.3f, 1.0f }),
          .c_colour = engine::SRGBToLinear({ 0.1f, 1.0f, 0.3f, 1.0f }),
          .uv_0 = { 0, 0 },
          .uv_1 = { 1, 0 },
          .uv_2 = { 0, 1 },
        };
        const auto e1 = spawn(r, "empty");
        give_life(r, e1, { 0, 0 }, { 0, 0 });
        r.remove<SpriteComponent>(e1);
        r.remove<TransformComponent>(e1);
        r.emplace<SpriteTriangleComponent>(e1, tri1);
        r.emplace<IslandShoreTriangle>(e1);

        auto& tag_c = r.get<TagComponent>(e1);
        tag_c.tag = "tri_shore";
      }
      // triangle 2: edge.a, pos_b, pos_a
      {
        SpriteTriangleComponent tri2 = {
          .a = b,
          .b = d,
          .c = c,
          .a_colour = engine::SRGBToLinear({ 0.8f, 0.0f, 0.0f, 1.0f }),
          .b_colour = engine::SRGBToLinear({ 0.1f, 1.0f, 0.3f, 1.0f }),
          .c_colour = engine::SRGBToLinear({ 0.1f, 1.0f, 0.3f, 1.0f }),
          .uv_0 = { 1, 0 },
          .uv_1 = { 1, 1 },
          .uv_2 = { 0, 1 },
        };
        const auto e2 = spawn(r, "empty");
        give_life(r, e2, { 0, 0 }, { 0, 0 });
        r.remove<SpriteComponent>(e2);
        r.remove<TransformComponent>(e2);
        r.emplace<SpriteTriangleComponent>(e2, tri2);
        r.emplace<IslandShoreTriangle>(e2);

        auto& tag_c = r.get<TagComponent>(e2);
        tag_c.tag = "tri_shore";
      }
    }
  }
}

void
hide_non_base_islands(entt::registry& r)
{
  const auto center_island_eid = get_center_island_eid(r);

  for (const auto& [e, island_c, bb_c] : r.view<DebugContoursComponent, const BoundingBoxComponent>().each()) {
    if (e == center_island_eid)
      continue; // dont hide base island

    // make all the non-base islands hidden.
    auto* hidden_c = r.try_get<IslandHiddenComponent>(e);
    if (!hidden_c)
      r.emplace<IslandHiddenComponent>(e);
  }

  {
    const auto view = r.view<IslandHiddenComponent>();
    SDL_Log("There are %d hidden islands.", view.size());
  }

  const auto view = r.view<const SpriteTriangleComponent, const HasParentComponent>();
  for (const auto& [e, triangle_c, parent_c] : view.each()) {

    const auto parent_e = parent_c.parent;

    // skip the center island
    if (parent_e == center_island_eid)
      continue;

    // add the hidden component to the triangles.
    r.emplace<IslandHiddenComponent>(e);
  }
};

void
create_island_floor_sprites(entt::registry& r) {
  /*
  const auto& islands_c = SINGLE_Islands::instance;
  const auto tilesize = islands_c.tilesize;
  const auto half_tilesize = tilesize * 0.5f;

  // floor sprites
  for (const auto [island_e, island_c, bb_c, contours_c] :
       r.view<const RockComponent, const BoundingBoxComponent, DebugContoursComponent>().each()) {

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

        // note: convert gp back to position
        // const auto spr_col = engine::SRGBColour{ 255, 255, 255, 100 };
        // const auto debug_e = spawn(r, "empty");
        // r.get<TagComponent>(debug_e).tag = "empty-IslandSquare";
        // give_life(r, debug_e, pos2, { 6, 6 });
        // set_sprite(r, debug_e, "CIRCLE");
        // set_colour(r, debug_e, spr_col);
        // set_z_index(r, debug_e, ZLayer::FLOOR);
      }
    }
  }
  */
};

void
create_above_island_sprites(entt::registry& r)
{
  const auto tilesize = SINGLE_Islands::instance.tilesize;

  const auto view = r.view<IslandHiddenComponent, const BoundingBoxComponent>();
  for (const auto& [e, hidden_c, aabb_c] : view.each()) {
    const auto center = 0.5f * (aabb_c.tl + aabb_c.br);

    // spawn a sprite above the island
    auto popup_e = spawn(r, "empty");
    give_life(r, popup_e, center, { tilesize, tilesize });
    set_sprite(r, popup_e, "TEXT_?");
    r.emplace<AboveHiddenComponent>(popup_e);

    hidden_c.island_popup_e = popup_e;
  }

  //
};

void
update_actor_rocks_system(entt::registry& r, glm::vec2 mouse_pos, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto rock_ui = gesert_menubar_state(menu_c, "DebugIslands");
  if (rock_ui.enabled) {
    ImGui::Begin("DebugIslands");
    imgui_draw_float("frequency", SINGLE_Islands::instance.frequency);
    imgui_draw_float("cutoff", SINGLE_Islands::instance.cutoff);
    // ImGui::Text("valid: %d", SINGLE_Islands::instance.valid);

    const auto wh = SINGLE_Islands::instance.wh;
    const auto tilesize = SINGLE_Islands::instance.tilesize;
    const auto offset = (int)(-wh * 0.5f);
    const auto offset_worldspace = (int)offset * tilesize;
    const glm::vec2 offset_mouse_pos = mouse_pos - glm::vec2{ offset_worldspace, offset_worldspace };

    const auto& generated = SINGLE_Islands::instance.generated;
    auto mouse_idx = engine::grid::worldspace_to_index({ offset_mouse_pos.x, offset_mouse_pos.y },
                                                       SINGLE_Islands::instance.tilesize,
                                                       SINGLE_Islands::instance.wh,
                                                       SINGLE_Islands::instance.wh);
    mouse_idx = glm::clamp(mouse_idx, 0, glm::max((int)generated.size() - 1, 0));

    ImGui::Text("mouse: %f, %f", mouse_pos.x, mouse_pos.y);
    ImGui::Text("offset mousepos: %f, %f", offset_mouse_pos.x, offset_mouse_pos.y);
    ImGui::Text("mouse idx: %i", mouse_idx);

    if (generated.size() > 0)
      ImGui::Text(
        "noise at (%i %i), idx: %f", generated[mouse_idx].xy.x, generated[mouse_idx].xy.y, generated[mouse_idx].noise);

    ImGui::End();
  }

#if defined(_DEBUG)
  auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_KP_7)) {
    // Destroy all the old rocks.
    for (const auto& [e, rock_c] : r.view<const RockComponent>().each())
      r.destroy(e);
    const auto view = r.view<TransformComponent>(entt::exclude<Persistent>);
    for (const auto& [e, t_c] : view.each())
      r.destroy(e);
#endif

    create_empty<RequestGenerateRocks>(r);
  }

  /*
static float im_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
static float im_green[4]{ 0.0f, 1.0f, 0.0f, 1.0f };
static float percent = 0.0f;
ImGui::ColorEdit4("red", im_red);
ImGui::ColorEdit4("green", im_green);
ImGui::SliderFloat("mix", &percent, 0.0f, 1.0f);

static engine::SRGBColour my_r;
static engine::SRGBColour my_g;
my_r = { im_red[0], im_red[1], im_red[2], im_red[3] };
my_g = { im_green[0], im_green[1], im_green[2], im_green[3] };
const auto lerped = lerp_colour(my_r, my_g, percent);

static float im_lerp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
im_lerp[0] = lerped.r / 255.0f;
im_lerp[1] = lerped.g / 255.0f;
im_lerp[2] = lerped.b / 255.0f;
im_lerp[3] = lerped.a / 255.0f;
ImGui::ColorEdit4("mixed_col", im_lerp);
*/

  process_requests<RequestGenerateRocks>(r, [&](const auto& req) {
    SDL_Log("Request to generate rocks...");

    // clear the id <=> eid map
    SINGLE_Islands::instance.id_to_island_eid.clear();

    generate_rocks(r);

    // draw_rocks()
    create_island_floor_sprites(r);
    create_island_triangles(r);
    hide_non_base_islands(r);
    create_above_island_sprites(r);
    create_shore_triangles(r);
  });

  // int j = 0;
  // const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  // static int debug_island = 0;
  // static int debug_edge = 0;
  // imgui_draw_int("debug_island", debug_island);
  // imgui_draw_int("debug_edge", debug_edge);
  // if (get_key_down(input_c, SDL_SCANCODE_KP_PLUS))
  //   debug_edge++;
  // if (get_key_down(input_c, SDL_SCANCODE_KP_MINUS))
  //   debug_edge--;

  /*
  {
    auto view = r.view<IslandEdgeLine, TransformComponent>();
    for (const auto& [e, edge_c, t_c] : view.each()) {
      edge_c.timer += dt;
      const auto& edge = edge_c.edge;
      const float thickness = edge_c.edge_size_base + sin(edge_c.timer * 2.0f) * 2.0f;
      const auto line_info = generate_line(edge.a, edge.b, thickness);
      set_position_and_size_with_line(r, e, line_info);
    }
  }
  */
}

} // namespace game2d