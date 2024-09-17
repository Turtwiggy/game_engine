#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "helpers/entity_pool.hpp"
#include "helpers/line.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "modules/system_spaceship_door/components.hpp"
#include "modules/vfx_grid/components.hpp"

#include "imgui.h"
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>
#include <glm/fwd.hpp>
#include <string>

namespace game2d {
using namespace std::literals;

const auto create_aabb = [](const glm::ivec2& tl, const int width, const int height) -> AABB {
  AABB aabb;
  aabb.center = { tl.x + (width / 2.0f), tl.y + (height / 2.0f) };
  aabb.size = { glm::abs(width), glm::abs(height) };
  return aabb;
};

std::optional<glm::ivec2>
line_intersection(const Wall& a, const Wall& b)
{
  return line_intersection(a.p0, a.p1, b.p0, b.p1);
};

entt::entity
create_door_along_two_points(entt::registry& r, const glm::ivec2& a, const glm::ivec2& b)
{
  const entt::entity door_e = create_gameplay(r, EntityType::solid_spaceship_door);

  const glm::vec2 diff = (b - a);
  const glm::vec2 midpoint = diff / 2.0f;
  const glm::ivec2 door_position = { a.x + midpoint.x, a.y + midpoint.y };
  set_position(r, door_e, door_position);

  auto& door_info = r.get<SpaceshipDoorComponent>(door_e);
  const int door_width = 4;
  glm::vec2 door_size = { 0, 0 };
  const bool is_horizontal = glm::abs(diff.x) > glm::abs(diff.y);
  if (is_horizontal)
    door_size = { glm::abs(diff.x), door_width };
  else
    door_size = { door_width, glm::abs(diff.y) };
  door_info.closed_size = door_size;
  set_size(r, door_e, door_size);

  // Create two pressure points either side, open and close
  //
  const glm::vec2 nrm_a = engine::normalize_safe({ -diff.y, diff.x });
  const glm::vec2 nrm_b = engine::normalize_safe({ diff.y, -diff.x });

  const auto pressureplate_e_0 = create_gameplay(r, EntityType::actor_spaceship_pressureplate);
  const auto pressureplate_e_1 = create_gameplay(r, EntityType::actor_spaceship_pressureplate);
  const auto pressureplate_e_2 = create_gameplay(r, EntityType::actor_spaceship_pressureplate);
  const auto pressureplate_e_3 = create_gameplay(r, EntityType::actor_spaceship_pressureplate);

  auto& pressureplate_0 = r.get<SpaceshipPressureplateComponent>(pressureplate_e_0);
  auto& pressureplate_1 = r.get<SpaceshipPressureplateComponent>(pressureplate_e_1);
  auto& pressureplate_2 = r.get<SpaceshipPressureplateComponent>(pressureplate_e_2);
  auto& pressureplate_3 = r.get<SpaceshipPressureplateComponent>(pressureplate_e_3);

  static engine::SRGBColour green{ 0.0f, 1.0f, 0.0f, 1.0f };
  static engine::SRGBColour red{ 1.0f, 0.0f, 0.0f, 1.0f };
  pressureplate_0.type = PressurePlateType::OPEN;
  pressureplate_1.type = PressurePlateType::CLOSE;
  pressureplate_2.type = PressurePlateType::OPEN;
  pressureplate_3.type = PressurePlateType::CLOSE;
  set_colour(r, pressureplate_e_0, green);
  set_colour(r, pressureplate_e_1, red);
  set_colour(r, pressureplate_e_2, green);
  set_colour(r, pressureplate_e_3, red);

  pressureplate_0.door = door_e;
  pressureplate_1.door = door_e;
  pressureplate_2.door = door_e;
  pressureplate_3.door = door_e;

  const float size = 8;
  const float dst = 10;
  const float value_away_from_center = (size / 2.0f) + 2; // size/2 + half_pixelgap

  glm::ivec2 dst_away_from_center{ 0, 0 };
  if (is_horizontal)
    dst_away_from_center = glm::ivec2{ value_away_from_center, 0 };
  else
    dst_away_from_center = glm::ivec2{ 0, value_away_from_center };

  // Positive side
  const glm::ivec2 pp0_pos = door_position + glm::ivec2(nrm_a * glm::vec2{ dst, dst }) - dst_away_from_center;
  const glm::ivec2 pp1_pos = door_position + glm::ivec2(nrm_a * glm::vec2{ dst, dst }) + dst_away_from_center;
  // negative side
  const glm::ivec2 pp2_pos = door_position + glm::ivec2(nrm_b * glm::vec2{ dst, dst }) - dst_away_from_center;
  const glm::ivec2 pp3_pos = door_position + glm::ivec2(nrm_b * glm::vec2{ dst, dst }) + dst_away_from_center;

  set_position(r, pressureplate_e_0, pp0_pos);
  set_position(r, pressureplate_e_1, pp1_pos);
  set_position(r, pressureplate_e_2, pp2_pos);
  set_position(r, pressureplate_e_3, pp3_pos);

  set_size(r, pressureplate_e_0, { size, size });
  set_size(r, pressureplate_e_1, { size, size });
  set_size(r, pressureplate_e_2, { size, size });
  set_size(r, pressureplate_e_3, { size, size });

  return door_e;
};

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::ivec2& input_mouse_pos, const float dt)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto toggle_l_align = SDL_SCANCODE_LEFT;
  const auto toggle_r_align = SDL_SCANCODE_RIGHT;
  const auto toggle_u_align = SDL_SCANCODE_UP;
  const auto toggle_d_align = SDL_SCANCODE_DOWN;
  //
  const auto drag_create_room_key = SDL_SCANCODE_R;
  const auto generate_dungeon_key = SDL_SCANCODE_G;
  const auto free_from_grid_key = SDL_SCANCODE_LSHIFT;
  const auto create_door_key = SDL_SCANCODE_D;
  const auto select_segment_key = SDL_SCANCODE_X;
  const auto delete_segments_key = SDL_SCANCODE_C;

  static int grid_snap_size = 50;
  ImGui::Begin("SpaceshipDesigner");
  imgui_draw_int("edit_gridsize", grid_snap_size);
  auto& grid_effect = get_first_component<Effect_GridComponent>(r);
  grid_effect.gridsize = grid_snap_size;

  ImGui::Text("free_from_grid_key: %s", SDL_GetScancodeName(free_from_grid_key));
  ImGui::Separator();
  ImGui::Text("drag_create_room_key: %s", SDL_GetScancodeName(drag_create_room_key));
  ImGui::Text("generate_dungeon_key: %s", SDL_GetScancodeName(generate_dungeon_key));
  ImGui::Text("create_door_key: %s", SDL_GetScancodeName(create_door_key));
  ImGui::Text("select_segment_key: %s", SDL_GetScancodeName(select_segment_key));
  ImGui::Text("delete_segments_key: %s", SDL_GetScancodeName(delete_segments_key));

  {
    const auto& spaceship_view = r.view<SpaceshipComponent>();
    if (spaceship_view.size() == 0) // ensure one spaceship
      const auto spaceship_e = create_gameplay(r, EntityType::actor_spaceship);
    // Display spaceship info
    for (const auto& [spaceship_e, spaceship_c] : spaceship_view.each())
      ImGui::Text("Spaceship");
  }

  glm::ivec2 mouse_pos = engine::grid::grid_space_to_world_space(
    engine::grid::world_space_to_grid_space(input_mouse_pos, grid_snap_size), grid_snap_size);

  // Adjust mouse based on grid-alignment
  {
    static int l_align = 0;
    static int r_align = 0;
    static int u_align = 0;
    static int d_align = 0;
    if (get_key_down(input, toggle_l_align))
      l_align = l_align == 0 ? 1 : 0;
    if (get_key_down(input, toggle_r_align))
      r_align = r_align == 0 ? 1 : 0;
    if (get_key_down(input, toggle_u_align))
      u_align = u_align == 0 ? 1 : 0;
    if (get_key_down(input, toggle_d_align))
      d_align = d_align == 0 ? 1 : 0;
    ImGui::Text("l_align %i", l_align);
    ImGui::Text("r_align %i", r_align);
    ImGui::Text("u_align %i", u_align);
    ImGui::Text("d_align %i", d_align);

    // Clamp mouse position to grid
    mouse_pos += glm::vec2(grid_snap_size / 2.0f, grid_snap_size / 2.0f); // center
    mouse_pos += (l_align == 1) ? glm::ivec2(-grid_snap_size / 2.0f, 0.0f) : glm::ivec2{ 0, 0 };
    mouse_pos += (r_align == 1) ? glm::ivec2(grid_snap_size / 2.0f, 0.0f) : glm::ivec2{ 0, 0 };
    mouse_pos += (u_align == 1) ? glm::ivec2(0.0f, -grid_snap_size / 2.0f) : glm::ivec2{ 0, 0 };
    mouse_pos += (d_align == 1) ? glm::ivec2(0.0f, grid_snap_size / 2.0f) : glm::ivec2{ 0, 0 };

    // Free the mouse from the grid
    if (get_key_held(input, free_from_grid_key))
      mouse_pos = input_mouse_pos;
  }

  // Debug mouse pos
  const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, grid_snap_size);
  ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);
  ImGui::Text("cursor_pos: %i %i", input_mouse_pos.x, input_mouse_pos.y);

  // Drag To Create Rooms
  //
  {
    static std::optional<glm::ivec2> press_location = std::nullopt;
    static entt::entity room_to_create;
    static std::vector<entt::entity> walls;
    static bool room_collision = false;

    if (get_key_down(input, drag_create_room_key)) {
      // set_keydown_state();
      press_location = mouse_pos;
      room_collision = false;

      // create room
      room_to_create = create_gameplay(r, EntityType::empty_with_transform);
      // create room colour
      set_colour(r, room_to_create, { 1.0f, 1.0f, 1.0f, 0.02f });
      r.emplace<Room>(room_to_create);

      // create physics actor
      r.emplace<PhysicsTransformXComponent>(room_to_create);
      r.emplace<PhysicsTransformYComponent>(room_to_create);
      r.emplace<AABB>(room_to_create);
      r.emplace<PhysicsActorComponent>(room_to_create);
      r.emplace<VelocityComponent>(room_to_create);

      // create room walls
      const auto w0 = create_wall(r, press_location.value(), glm::ivec2{ 0, 0 }, room_to_create);
      const auto w1 = create_wall(r, press_location.value(), glm::ivec2{ 0, 0 }, room_to_create);
      const auto w2 = create_wall(r, press_location.value(), glm::ivec2{ 0, 0 }, room_to_create);
      const auto w3 = create_wall(r, press_location.value(), glm::ivec2{ 0, 0 }, room_to_create);
      walls = { w0, w1, w2, w3 };
    }
    if (get_key_held(input, drag_create_room_key) && press_location.has_value()) {

      const auto& p = press_location.value();
      // width & height can be negative
      const int width = mouse_pos.x - p.x;
      const int height = mouse_pos.y - p.y;
      ImGui::Text("Width: %i Height: %i", width, height);

      // set mouse-cursor as top-left
      const glm::ivec2 tl = p;
      const glm::ivec2 tr = { p.x + width, p.y };
      const glm::ivec2 bl = { p.x, p.y + height };
      const glm::ivec2 br = { p.x + width, p.y + height };

      r.replace<Wall>(walls[0], Wall{ tl, tr, room_to_create });
      r.replace<Wall>(walls[1], Wall{ tr, br, room_to_create });
      r.replace<Wall>(walls[2], Wall{ br, bl, room_to_create });
      r.replace<Wall>(walls[3], Wall{ bl, tl, room_to_create });

      // Check if room collides with another existing room
      const auto create_aabb_from_mouse = [&mouse_pos](const glm::ivec2& p) -> AABB {
        int width = mouse_pos.x - p.x; // width & height can be negative
        int height = mouse_pos.y - p.y;
        if (width == 0)
          width = 1;
        if (height == 0)
          height = 1;
        return create_aabb(p, width, height);
      };

      // This here seems wrong, like it's bypassing the physics system.
      //
      const auto temp_aabb = create_aabb_from_mouse(press_location.value());
      set_position(r, room_to_create, temp_aabb.center);
      set_size(r, room_to_create, temp_aabb.size);
      for (const auto& [room_e, other_room_c, other_room_aabb] : r.view<Room, AABB>().each()) {
        if (room_e == room_to_create)
          continue; // dont self collide
        room_collision |= collide(temp_aabb, other_room_aabb);
      }
    }
    if (get_key_up(input, drag_create_room_key)) {

      // cleanup invalid room
      //
      if (room_collision) {
        dead.dead.emplace(room_to_create);
        for (const auto& wall : walls)
          dead.dead.emplace(wall);
        walls.clear();
      }

      // set_keyup_state();
      press_location = std::nullopt;
      room_collision = false;
    }
  }

  // Convert the gridbox to lines
  //
  AABB aabb_gridbox;
  std::vector<Wall> aabb_gridbox_to_lines;
  {
    // Create an AABB for the gridbox.
    const glm::ivec2 grid_tl_worldspace = { grid_pos.x * grid_snap_size, grid_pos.y * grid_snap_size };
    aabb_gridbox = create_aabb(grid_tl_worldspace, grid_snap_size, grid_snap_size);
    // Convert Gridbox's AABB to 4 Lines.
    const auto& p = aabb_gridbox.center;
    const int half_width = static_cast<int>(aabb_gridbox.size.x / 2.0f);
    const int half_height = static_cast<int>(aabb_gridbox.size.y / 2.0f);
    const glm::ivec2 tl = { p.x - half_width, p.y - half_height };
    const glm::ivec2 tr = { p.x + half_width, p.y - half_height };
    const glm::ivec2 bl = { p.x - half_width, p.y + half_height };
    const glm::ivec2 br = { p.x + half_width, p.y + half_height };
    aabb_gridbox_to_lines = { { tl, tr }, { tr, br }, { br, bl }, { bl, tl } };
  }

  // Select Walls that intersect with the GridBox & Get their intersection points
  //
  if (get_key_down(input, select_segment_key)) {

    for (const auto& [wall_e, wall_c] : r.view<Wall>().each()) {
      wall_c.intersections.clear();

      // Create an AABB for the wall (line).
      //
      const auto pos_l = wall_c.p0.x <= wall_c.p1.x ? wall_c.p0.x : wall_c.p1.x;
      const auto pos_t = wall_c.p0.y <= wall_c.p1.y ? wall_c.p0.y : wall_c.p1.y;
      const auto line_tl = glm::ivec2{ pos_l, pos_t };
      // Fix: if a line is directly horizontal, height would be 0, meaning no collision
      const int start_width = glm::abs(wall_c.p1.x - wall_c.p0.x);
      const int start_height = glm::abs(wall_c.p1.y - wall_c.p0.y);
      const int width = (start_width == 0 && start_height != 0) ? 1 : start_width;
      const int height = (start_height == 0 && start_width != 0) ? 1 : start_height;
      const auto aabb_wall_line = create_aabb(line_tl, width, height);

      // Check if the two collide.
      if (!collide(aabb_wall_line, aabb_gridbox))
        continue;

      // With a Line-Line Intersection Algorithm,
      // Figure out the intersection point of:
      // the grid-box boundary & the wall-section.
      //

      for (const auto& aabb_line : aabb_gridbox_to_lines) {
        const Wall& l0 = aabb_line; // grid box, made of 2 points
        const Wall& l1 = wall_c;    // wall line, made of 2 points
        const auto maybe_intersection_point = line_intersection(l0, l1);
        if (!maybe_intersection_point.has_value())
          continue; // no intersection point

        const auto i = maybe_intersection_point.value();
        ImGui::Text("Valid Intersection: %i %i", i.x, i.y);

        // Create Wall Entry in Map
        wall_c.intersections.push_back(i);
      }
    }
  }

  //
  // Delete/Update Segments based on if theyre:
  // an end-line-segment, (1 collision?)
  // a middle-line-segment (2 collisions?)
  //
  const bool clip = get_key_down(input, delete_segments_key);
  std::vector<std::tuple<entt::entity, glm::ivec2, glm::ivec2>> clipped_pairs;

  for (const auto& [wall_e, wall_c] : r.view<Wall>().each()) {
    const auto& intersections = wall_c.intersections;
    ImGui::Text("Wall: intersections: %i", intersections.size());
    for (const auto& inter : intersections)
      ImGui::Text("Intersection: %i %i", inter.x, inter.y);

    // Likely on a corner
    //
    if (intersections.size() == 1) {
      ImGui::Text("On a wall line; probably an end piece.");
      // Shorten the end piece to the intersection point
      const auto& intersection = intersections[0];

      if (clip) {
        // work out which point to set
        // set the point thats closer to the intersection?
        const auto p0 = wall_c.p0;
        const auto p1 = wall_c.p1;
        const auto p0_dir = p0 - intersection;
        const auto p1_dir = p1 - intersection;
        const auto p0_d2 = p0_dir.x * p0_dir.x + p0_dir.y * p0_dir.y;
        const auto p1_d2 = p1_dir.x * p1_dir.x + p1_dir.y * p1_dir.y;

        if (p0_d2 < p1_d2)
          wall_c.p0 = intersection;
        else
          wall_c.p1 = intersection;
      }
    }

    // Likely in the middle line(s)
    //
    if (intersections.size() == 2) {
      ImGui::Text("On a wall line; probably a middle piece.");
      const auto& i0 = intersections[0];
      const auto& i1 = intersections[1];
      const auto& p0 = wall_c.p0;
      const auto& p1 = wall_c.p1;

      if (clip) {
        const auto get_d2 = [](const glm::ivec2& a, const glm::ivec2& b) -> int {
          const glm::ivec2 dir = b - a;
          return dir.x * dir.x + dir.y * dir.y;
        };
        const auto p0_i0_d2 = get_d2(p0, i0);
        const auto p1_i0_d2 = get_d2(p1, i0);
        const auto p0_i1_d2 = get_d2(p0, i1);
        const auto p1_i1_d2 = get_d2(p1, i1);

        // Here's the situation.
        // We have a line, containing p0 and p1.
        // We have two intersection points, i0 and i1.
        // We're interested in creating 2 new lines that make the most sense.

        // From intersection point 0, choose the point thats closest to it.
        // From intersection point 1, choose the points thats closest to it.
        glm::ivec2 px_closer_to_i0 = p0_i0_d2 < p1_i0_d2 ? p0 : p1;
        glm::ivec2 px_closer_to_i1 = p0_i1_d2 < p1_i1_d2 ? p0 : p1;

        // Here, we're in trouble if both intersection points have chosen the same (p0 or p1).
        // This is a common case. The intersection point that should have priority
        // over the chosen point is the intersection point that is closer.
        //
        if (px_closer_to_i0 == px_closer_to_i1) {

          // given both intersections chose the same px,
          // which intersection did i0 choose?
          const bool both_ix_chose_p0 = p0_i0_d2 < p1_i0_d2;

          if (both_ix_chose_p0) {

            // which intersection is closer to p0?
            const bool i0_closer_to_p0 = p0_i0_d2 < p0_i1_d2;

            // if i0 is closer to p0, it wins (p0).
            // if i1 is closer to p0, it wins (p0).
            px_closer_to_i0 = i0_closer_to_p0 ? p0 : p1;
            px_closer_to_i1 = i0_closer_to_p0 ? p1 : p0;
          }
          // both chosen p1
          else {
            // which intersection is closer to p1?
            const bool i0_closer_to_p1 = p1_i0_d2 < p1_i1_d2;

            // if i0 is closer to p1, it wins (p1).
            // if i1 is closer to p1, it wins (p1).
            px_closer_to_i0 = i0_closer_to_p1 ? p1 : p0;
            px_closer_to_i1 = i0_closer_to_p1 ? p0 : p1;
          }
        }

        // Create & Update Existing Line
        wall_c.p0 = i0;
        wall_c.p1 = px_closer_to_i0;

        // Create & Update New Line
        const auto new_wall_e = create_gameplay(r, EntityType::solid_wall);
        Wall new_line;
        new_line.p0 = i1;
        new_line.p1 = px_closer_to_i1;
        // new_line.parent_room = wall_c.parent_room;
        r.emplace<Wall>(new_wall_e, new_line);

        clipped_pairs.push_back(std::make_tuple(wall_c.parent_room, i0, i1));
      }
    }
  }

  // All intersections are now out of date
  if (clip) {
    for (const auto& [wall_e, wall_c] : r.view<Wall>().each())
      wall_c.intersections.clear();
  }

  //
  // Create the door based on intersection points
  //
  for (const auto& [room, i0, i1] : clipped_pairs) {
    const auto door_e = create_door_along_two_points(r, i0, i1);
    auto& door_c = r.get<SpaceshipDoorComponent>(door_e);
    door_c.parent_room = room;
  }
  clipped_pairs.clear();

  // Render all walls
  const auto& wall_view = r.view<Wall>();
  for (const auto& [wall_e, wall_c] : wall_view.each())
    set_position_and_size_with_line(r, wall_e, generate_line(wall_c.p0, wall_c.p1, 1));

  // Render points of intersection
  {
    const auto debug_points_pool_e = get_first<EntityPool>(r);
    if (debug_points_pool_e == entt::null)
      r.emplace<EntityPool>(create_gameplay(r, EntityType::empty_no_transform));
    auto& debug_points_pool_c = get_first_component<EntityPool>(r);

    // Create debug entities
    size_t total_intersections = 0;
    for (const auto& [wall_e, wall_c] : wall_view.each())
      total_intersections += wall_c.intersections.size();
    debug_points_pool_c.update(r, static_cast<int>(total_intersections));

    // Position debug entities
    for (int i = 0; const auto& [wall_e, wall_c] : wall_view.each()) {
      for (const auto& intersection : wall_c.intersections) {
        const auto debug_e = debug_points_pool_c.instances[i++];
        set_position(r, debug_e, { intersection.x, intersection.y });
        set_size(r, debug_e, { 6, 6 });
      }
    }
  }
}

} // namespace game2d
