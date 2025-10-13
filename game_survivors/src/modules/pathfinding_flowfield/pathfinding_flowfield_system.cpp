#include "pch.hpp"

#include "pathfinding_flowfield_components.hpp"
#include "pathfinding_flowfield_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/priority_queue.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/std/unordered_set/glm_hash.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_spawner/spawner_system.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
generate_flowfield(entt::registry& r, const GenerateFlowfieldParams in)
{
  const auto& map_c = get_first_component<MapComponent>(r);
  auto& ff_c = get_first_component<SINGLE_Flowfield>(r);

  // params
  const int xmax = in.xmax;
  const int ymax = in.ymax;
  const int from = in.from_idx;
  const auto from_gp_pair = engine::grid::index_to_grid_position(from, xmax);
  const auto from_gp = vec2i{ from_gp_pair.first, from_gp_pair.second };

  PriorityQueue<int> frontier;
  frontier.enqueue(from, 0);
  std::map<int, int> came_from;
  std::map<int, int> cost_so_far; // index, distance
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (!frontier.empty()) {
    const int current = static_cast<int>(frontier.dequeue());
    const auto gpos = engine::grid::index_to_grid_position(current, xmax);

    // if (gpos.first == 1 & gpos.second == 0)
    //   int k = 1;

    const auto n_idxs = engine::grid::get_neighbour_gridpos({ gpos.first, gpos.second });
    for (const auto& [n_dir, n_gp] : n_idxs) {
      const auto n_idx = engine::grid::grid_position_to_index(n_gp, xmax);

      if (gp_out_of_bounds(n_gp, xmax, ymax))
        continue; // out of map

      const bool blocked = get_cost_at_gridpos(r, n_gp, map_c) < 0;
      if (blocked)
        continue; // impassable

      const int neighbour_cost = 1;
      const int new_cost = cost_so_far[current] + neighbour_cost;

      if (!cost_so_far.contains(n_idx) || new_cost < cost_so_far[n_idx]) {
        cost_so_far[n_idx] = new_cost;
        const auto priority = new_cost + heuristic<vec2i>(vec2i{ n_gp.x, n_gp.y }, from_gp);
        frontier.enqueue(n_idx, priority);
        came_from[n_idx] = current;
      }
    }
  }

  // assign the results
  ff_c.came_from = came_from;

  // construct the "goes to" map.
  // for (const auto& [a, b] : came_from)
  //   ff_c.goes_to[b] = a;

  SDL_Log("Flowfield generated. From: %i", from);
};

static int tilesize = 32;
static int xmax = 30;
static auto from_gp = glm::ivec2{ 0, 0 };
static auto fixed_mouse_gp = glm::ivec2{ 0, 0 };
static bool use_mouse_pos = true;

void
request_generate_flowfield(entt::registry& r, glm::vec2 mouse_pos)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map_c = r.get<MapComponent>(map_e);
  auto& ff_c = get_first_component<SINGLE_Flowfield>(r);

  map_c.tilesize = tilesize;
  map_c.xmax = xmax;
  map_c.ymax = xmax;
  map_c.map.resize(map_c.xmax * map_c.ymax);
  ff_c.came_from.clear();

  const auto mouse_gp = engine::grid::worldspace_to_clamped_gridspace(mouse_pos, map_c.tilesize, map_c.xmax, map_c.ymax);
  const auto mouse_idx = engine::grid::grid_position_to_index(mouse_gp, map_c.xmax);
  const auto imgui_idx = engine::grid::grid_position_to_clamped_index(from_gp, map_c.xmax, map_c.ymax);

  const RequestGenerateFlowField req{
    .params =
      GenerateFlowfieldParams{
        .xmax = xmax,
        .ymax = xmax,
        .tilesize = tilesize,
        .from_idx = use_mouse_pos ? mouse_idx : imgui_idx,
      },
  };

  create_empty<RequestGenerateFlowField>(r, req);
};

void
update_imgui_flowfield_ui(entt::registry& r, glm::vec2 mouse_pos)
{
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto demo_state = gesert_menubar_state(menu_c, "FlowField");
  if (!demo_state.enabled)
    return;

  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map_c = r.get<MapComponent>(map_e);
  auto& ff_c = get_first_component<SINGLE_Flowfield>(r);

  ImGui::Begin("DebugFlowField");

  imgui_draw_int("tilesize", tilesize);
  imgui_draw_int("xmax", xmax);
  imgui_draw_ivec2("from_gp", from_gp);
  imgui_draw_bool("use_mouse_pos", use_mouse_pos);

  if (ImGui::Button("Generate"))
    request_generate_flowfield(r, mouse_pos);

  ImGui::End();
}

void
update_generate_flow_field_system(entt::registry& r, glm::vec2 mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

#if defined(_DEBUG)

  const auto offset = glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
  const auto mouse_gp = engine::grid::worldspace_to_clamped_gridspace(mouse_pos, tilesize, xmax, xmax);
  const auto mouse_idx = engine::grid::grid_position_to_index(mouse_gp, xmax);

  draw_sprite(r,
              Sprite{
                .sprite = "SKULL_AND_BONES",
                .pos = engine::grid::gridspace_to_worldspace(mouse_gp, tilesize),
                .size = { 16, 16 },
                .z_idx = ZLayer::FOREGROUND,
              });

  draw_sprite(r,
              Sprite{
                .sprite = "SKULL_AND_BONES",
                .pos = engine::grid::gridspace_to_worldspace(from_gp, tilesize),
                .size = { 16, 16 },
                .z_idx = ZLayer::FOREGROUND,
              });

  // when the mouse changes gridpos, update flowfield
  if (fixed_mouse_gp != mouse_gp) {
    fixed_mouse_gp = mouse_gp;
    request_generate_flowfield(r, mouse_pos);
  }

  // generate flowfield if kp pressed
  auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_KP_4)) {
    SDL_Log("Request generate flowfield... (to mouse position)");
    request_generate_flowfield(r, mouse_pos);
  }

  update_imgui_flowfield_ui(r, mouse_pos);

  // spawn an enemy that follows the flowfield.
  if (get_key_down(input_c, SDL_SCANCODE_KP_5)) {
    // todo
    auto e = spawn_enemy(r, "actor_enemy_flow", 10);
    set_position(r, e, { 0, 0 });
  }

#endif

  process_requests<RequestGenerateFlowField>(r, [&](const auto& req) {
    //
    generate_flowfield(r, req.params.value());
  });
}

void
update_display_flow_field_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);
  const auto& ff_c = get_first_component<SINGLE_Flowfield>(r);

  if (ff_c.came_from.empty())
    return; // no flowfield generated

  const std::unordered_map<glm::ivec2, std::string> spritemap{
    { glm::ivec2{ 0, -1 }, "ARROW_UP" },
    { glm::ivec2{ 0, 1 }, "ARROW_DOWN" },
    { glm::ivec2{ 1, 0 }, "ARROW_RIGHT" },
    { glm::ivec2{ -1, 0 }, "ARROW_LEFT" },
  };

  for (const auto& [a, b] : ff_c.came_from) {
    const auto a_pos = engine::grid::index_to_world_position(a, map_c.xmax, map_c.ymax, map_c.tilesize);
    const auto a_gp = engine::grid::index_to_grid_position(a, map_c.xmax);
    const auto b_gp = engine::grid::index_to_grid_position(b, map_c.xmax);

    auto dir = glm::ivec2{ b_gp.first - a_gp.first, b_gp.second - a_gp.second };
    // dir.x = dir.x > 0 ? 1 : dir.x;
    // dir.x = dir.x < 0 ? -1 : dir.x;
    // dir.y = dir.y > 0 ? 1 : dir.y;
    // dir.y = dir.y < 0 ? -1 : dir.y;

    if (dir == glm::ivec2{ 0, 0 }) {
      const Sprite spr{
        .sprite = "CIRCLE",
        .pos = glm::vec2{ a_pos.first, a_pos.second },
        .size = { map_c.tilesize, map_c.tilesize },
        .col = { 0.0f, 1.0f, 0.0f, 1.0f },
      };
      draw_sprite(r, spr);
      continue;
    }

    const Sprite spr{
      .sprite = spritemap.at(dir),
      .pos = glm::vec2{ a_pos.first, a_pos.second },
      .size = { map_c.tilesize, map_c.tilesize },
      .col = { 1.0f, 0.0f, 0.0f, 1.0f },
    };
    draw_sprite(r, spr);
  }
}

} // namespace game2d