#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/system_spaceship_door/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"

#include "imgui.h"
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

namespace game2d {
using namespace std::literals;

const auto create_aabb = [](const glm::ivec2& tl, const int width, const int height) -> AABB {
  AABB aabb;
  aabb.center = { tl.x + (width / 2.0f), tl.y + (height / 2.0f) };
  aabb.size = { glm::abs(width), glm::abs(height) };
  return aabb;
};

// Walls are basically "Lines"
struct Wall
{
  glm::ivec2 p0{ 0, 0 };
  glm::ivec2 p1{ 0, 0 };
  std::vector<glm::ivec2> intersections; // storage for when this wall is being intersected
};

static int global_room_id = 0;
struct Room
{
  int id = 0;
  std::vector<entt::entity> walls;
  AABB aabb;

  Room() { id = global_room_id++; };
};

// Function to calculate the intersection point of two lines
// Returns a tuple with the intersection point (x, y)
std::optional<glm::ivec2>
line_intersection(const int x1, // p0
                  const int y1,
                  const int x2, // p1
                  const int y2,
                  const int x3, // p2
                  const int y3,
                  const int x4, // p3
                  const int y4)
{
  // Calculate determinants
  const int det = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  const int det_x = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
  const int det_y = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);

  // If determinant is zero, lines are parallel or coincident
  // If determinant is negative... not intested in future collisions.
  if (det == 0)
    return std::nullopt;

  // Calculate intersection point
  const float intersection_x = det_x / static_cast<float>(det);
  const float intersection_y = det_y / static_cast<float>(det);
  const glm::ivec2 intersection = { static_cast<int>(intersection_x), static_cast<int>(intersection_y) };

  ImGui::Separator();
  ImGui::Text("Potential Intersection! %i %i", intersection.x, intersection.y);

  // check the intersection lies within the line segments.
  AABB point;
  point.center = intersection;
  point.size = { 1, 1 };
  // ImGui::Text("Point. Centre: %i %i. Size: %i %i", point.center.x, point.center.y, point.size.x, point.size.y);

  const int l0 = glm::min(x1, x2);
  const int r0 = glm::max(x1, x2);
  const int t0 = glm::min(y1, y2);
  const int b0 = glm::max(y1, y2);
  AABB line0 = create_aabb({ l0, t0 }, glm::abs(r0 - l0), glm::abs(b0 - t0));
  line0.size.x = line0.size.x == 0 ? 1 : line0.size.x;
  line0.size.y = line0.size.y == 0 ? 1 : line0.size.y;
  // ImGui::Text("line0. Centre: %i %i. Size: %i %i", line0.center.x, line0.center.y, line0.size.x, line0.size.y);

  const int l = glm::min(x3, x4);
  const int r = glm::max(x3, x4);
  const int t = glm::min(y3, y4);
  const int b = glm::max(y3, y4);
  AABB line1 = create_aabb({ l, t }, glm::abs(r - l), glm::abs(b - t));
  line1.size.x = line1.size.x == 0 ? 1 : line1.size.x;
  line1.size.y = line1.size.y == 0 ? 1 : line1.size.y;
  // ImGui::Text("line1. Centre: %i %i. Size: %i %i", line1.center.x, line1.center.y, line1.size.x, line1.size.y);

  const bool coll_a = collide(point, line0);
  const bool coll_b = collide(point, line1);
  ImGui::Text("coll_a %i coll_b %i", coll_a, coll_b);
  if (coll_a && coll_b)
    return intersection;
  return std::nullopt;
}

std::optional<glm::ivec2>
line_intersection(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3)
{
  return line_intersection(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
};

std::optional<glm::ivec2>
line_intersection(const Wall& a, const Wall& b)
{
  return line_intersection(a.p0, a.p1, b.p0, b.p1);
};

void
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

  pressureplate_0.type = PressurePlateType::OPEN;
  pressureplate_1.type = PressurePlateType::CLOSE;
  pressureplate_2.type = PressurePlateType::OPEN;
  pressureplate_3.type = PressurePlateType::CLOSE;

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
};

entt::entity
create_wall(entt::registry& r, const glm::ivec2 pos, Room& room)
{
  const auto wall_e = create_gameplay(r, EntityType::solid_wall);
  r.emplace<Wall>(wall_e);
  set_position(r, wall_e, pos);
  set_size(r, wall_e, { 16, 16 });
  room.walls.push_back(wall_e);
  return wall_e;
};

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::ivec2& input_mouse_pos)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto toggle_l_align = SDL_SCANCODE_LEFT;
  const auto toggle_r_align = SDL_SCANCODE_RIGHT;
  const auto toggle_u_align = SDL_SCANCODE_UP;
  const auto toggle_d_align = SDL_SCANCODE_DOWN;
  //
  const auto create_point_key = SDL_SCANCODE_C;
  const auto join_points_key = SDL_SCANCODE_F;
  const auto delete_points_key = SDL_SCANCODE_DELETE;
  //
  const auto drag_create_room_key = SDL_SCANCODE_R;
  const auto generate_dungeon_key = SDL_SCANCODE_G;
  const auto free_from_grid_key = SDL_SCANCODE_LSHIFT;
  const auto create_door_key = SDL_SCANCODE_D;
  const auto delete_segment_key = SDL_SCANCODE_X;

  static int grid_snap_size = 50;
  ImGui::Begin("SpaceshipDesigner");
  imgui_draw_int("edit_gridsize", grid_snap_size);
  auto& grid_effect = get_first_component<Effect_GridComponent>(r);
  grid_effect.gridsize = grid_snap_size;

  ImGui::Text("free_from_grid_key: %s", SDL_GetScancodeName(free_from_grid_key));
  ImGui::Separator();
  ImGui::Text("create_point_key: %s", SDL_GetScancodeName(create_point_key));
  ImGui::Text("join_points_key: %s", SDL_GetScancodeName(join_points_key));
  ImGui::Text("delete_points_key: %s", SDL_GetScancodeName(delete_points_key));
  ImGui::Separator();
  ImGui::Text("drag_create_room_key: %s", SDL_GetScancodeName(drag_create_room_key));
  ImGui::Text("generate_dungeon_key: %s", SDL_GetScancodeName(generate_dungeon_key));
  ImGui::Text("create_door_key: %s", SDL_GetScancodeName(create_door_key));
  ImGui::Text("delete_segment_key: %s", SDL_GetScancodeName(delete_segment_key));

  const auto& spaceship_view = r.view<SpaceshipComponent>(entt::exclude<WaitForInitComponent>);
  if (spaceship_view.size_hint() == 0) // ensure one spaceship
    const auto spaceship_e = create_gameplay(r, EntityType::actor_spaceship);

  // Display spaceship info
  for (const auto& [spaceship_e, spaceship_c] : spaceship_view.each())
    ImGui::Text("Spaceship");

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

  const auto create_aabb_from_mouse = [&mouse_pos](const glm::ivec2& p) -> AABB {
    // width & height can be negative
    const int width = mouse_pos.x - p.x;
    const int height = mouse_pos.y - p.y;
    return create_aabb(p, width, height);
  };

  // Drag To Create Rooms
  //
  {
    const int line_width = 1;
    static std::optional<glm::ivec2> press_location = std::nullopt;
    static Room room_to_create;
    static bool room_collision = false;
    ImGui::Text("Room Collision: %i", room_collision);

    if (get_key_down(input, drag_create_room_key) && press_location == std::nullopt) {
      // set_keydown_state();
      press_location = mouse_pos;
      room_collision = false;
      for (const auto& wall : room_to_create.walls)
        dead.dead.emplace(wall);
      room_to_create = Room();

      // create 4 walls
      for (int i = 0; i < 4; i++)
        create_wall(r, press_location.value(), room_to_create);
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

      r.emplace_or_replace<Wall>(room_to_create.walls[0], Wall{ tl, tr });
      r.emplace_or_replace<Wall>(room_to_create.walls[1], Wall{ tr, br });
      r.emplace_or_replace<Wall>(room_to_create.walls[2], Wall{ br, bl });
      r.emplace_or_replace<Wall>(room_to_create.walls[3], Wall{ bl, tl });

      // Set visuals for the room-to-create
      for (const entt::entity& e : room_to_create.walls) {
        const auto& wall_c = r.get<Wall>(e);
        set_position_and_size_with_line(r, e, generate_line(wall_c.p0, wall_c.p1, line_width));
      }

      // Check if room collides with another existing room
      const AABB r1 = create_aabb_from_mouse(press_location.value());
      const auto& rooms_view = r.view<Room>(entt::exclude<WaitForInitComponent>);
      for (const auto& [room_e, room] : rooms_view.each())
        room_collision |= collide(r1, room.aabb);
    }
    if (get_key_up(input, drag_create_room_key)) {

      // Clean up old room
      // create_doors_at_collision
      if (room_collision) {
        for (const auto& wall : room_to_create.walls)
          dead.dead.emplace(wall);
      }

      // finalize room
      if (!room_collision) {
        const auto e = create_gameplay(r, EntityType::empty_no_transform);
        r.emplace<Room>(e, std::move(room_to_create));
      }

      // set_keyup_state();
      press_location = std::nullopt;
      room_collision = false;
    }
  }

  // Debug the hovered grid position
  // This is pretty much a grid-cursor
  //
  AABB aabb_gridbox;
  std::vector<Wall> aabb_gridbox_to_lines;
  {
    // static std::vector<entt::entity> wall_debug_lines;
    // if (wall_debug_lines.size() == 0) {
    //   for (int i = 0; i < 4; i++) {
    //     const auto e = create_gameplay(r, EntityType::empty_with_transform);
    //     set_position(r, e, mouse_pos);
    //     set_size(r, e, { 16, 16 });
    //     wall_debug_lines.push_back(e);
    //   }
    // }

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

    // debug the grid box
    // for (int i = 0; i < 4; i++) {
    //   const auto e = wall_debug_lines[i];
    //   const auto line = aabb_gridbox_to_lines[i];
    //   set_position_and_size_with_line(r, e, generate_line(line.p0, line.p1, 1));
    // }
  }

  // Work out which wall segments to delete
  //
  if (get_key_down(input, delete_segment_key)) {

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

  // Debug points of intersection
  //
  {
    static std::vector<entt::entity> debug_coll_points;
    for (int i = 0; const auto& [wall_e, wall_c] : r.view<Wall>().each()) {
      for (const auto& intersection : wall_c.intersections) {
        // create a new debug point
        if (debug_coll_points.size() == i)
          debug_coll_points.push_back(create_gameplay(r, EntityType::empty_with_transform));
        const auto& e = debug_coll_points[i];
        set_position(r, e, { intersection.x, intersection.y });
        set_size(r, e, { 6, 6 });
        i++;
      }
    }
  }

  // HACK: only do the delete if key pressed
  static bool clip = false;
  clip = get_key_down(input, SDL_SCANCODE_P);
  std::vector<glm::ivec2> intersection_points;

  //
  // Separate/Update the Wall Segments
  // work out if lines of interest collide with intersection points in an end-line-segment or middle-line-segment case.
  //
  for (const auto& [room_e, room_c] : r.view<Room>().each()) {
    for (const entt::entity& wall_ent : room_c.walls) {
      const auto* maybe_wall_c = r.try_get<Wall>(wall_ent);
      if (maybe_wall_c == nullptr)
        continue;
      auto& wall_c = r.get<Wall>(wall_ent);
      const std::vector<glm::ivec2>& intersections = wall_c.intersections;
      ImGui::Text("Wall: intersections: %i", intersections.size());
      const auto p0 = wall_c.p0;
      const auto p1 = wall_c.p1;

      // Likely on a corner
      //
      if (intersections.size() == 1) {
        ImGui::Text("On a wall line; probably an end piece.");
        // Shorten the end piece to the intersection point
        const auto& intersection = intersections[0];

        if (clip) {
          // work out which point to set
          // set the point thats closer to the intersection?
          const auto p0_dir = p0 - intersection;
          const auto p1_dir = p1 - intersection;
          const auto p0_d2 = p0_dir.x * p0_dir.x + p0_dir.y * p0_dir.y;
          const auto p1_d2 = p1_dir.x * p1_dir.x + p1_dir.y * p1_dir.y;

          if (p0_d2 < p1_d2)
            wall_c.p0 = intersection;
          else
            wall_c.p1 = intersection;

          set_position_and_size_with_line(r, wall_ent, generate_line(wall_c.p0, wall_c.p1, 1));
        }
      }

      // Likely in the middle line(s)
      //
      if (intersections.size() == 2) {
        ImGui::Text("On a wall line; probably a middle piece.");
        const auto& i0 = intersections[0];
        const auto& i1 = intersections[1];

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
            const bool both_ix_chose_p0 = p0_i0_d2 < p1_i0_d2; // given that they both chose the closer
            if (both_ix_chose_p0) {
              if (p0_i0_d2 < p0_i1_d2) // ding ding: i0 wins, as it's closer to p0. Sorry i1.
              {
                px_closer_to_i1 = p1;
              }
            }
            // both chosen p1
            else {
              if (p1_i0_d2 < p1_i1_d2) // ding ding: i0 wins, as it's closer to p1. Sorry i1.
              {
                px_closer_to_i1 = p0;
              }
            }
          }

          // Create & Update Existing Line
          set_position_and_size_with_line(r, wall_ent, generate_line(i0, px_closer_to_i0, 1));
          wall_c.p0 = i0;
          wall_c.p1 = px_closer_to_i0;

          // Create & Update New Line
          const auto new_wall_e = create_gameplay(r, EntityType::solid_wall);
          set_position_and_size_with_line(r, new_wall_e, generate_line(i1, px_closer_to_i1, 1));
          Wall new_line;
          new_line.p0 = i1;
          new_line.p1 = px_closer_to_i1;
          r.emplace<Wall>(new_wall_e);

          // Add new wall to the room
          room_c.walls.push_back(new_wall_e);

          // Where to add door
          intersection_points = { i0, i1 };
        }
      }
    }
  }

  // Should be auto, not on "clip"
  const bool create_door = true;
  if (create_door && clip && intersection_points.size() == 2)
    create_door_along_two_points(r, intersection_points[0], intersection_points[1]);

  // done the work
  if (clip)
    clip = false;

  // Misc
  //
  {
    // HACK: Spawn a Player
    if (get_mouse_rmb_press()) {
      const auto player = create_gameplay(r, EntityType::actor_player);
      set_position(r, player, mouse_pos);
    }

    // Debug Rooms
    const auto& rooms_view = r.view<Room>();
    for (const auto& [room_e, room_c] : rooms_view.each())
      ImGui::Text("RoomExists! Walls: %i", room_c.walls.size());

    // Generate dungeon base
    if (get_key_down(input, generate_dungeon_key)) {
      // Delete old Dungeon
      for (const auto& [room_e, room_c] : rooms_view.each()) {
        for (const entt::entity& w : room_c.walls)
          dead.dead.emplace(w);
        dead.dead.emplace(room_e);
      }

      const int width = 50;
      const int height = 50;
      const int seed = 0;
      engine::RandomState rnd;
      rnd.rng.seed(seed);

      std::vector<bool> walls_or_floors; // wall = true
      walls_or_floors.resize(width * height);

      // Create all as walls
      for (int xy = 0; xy < width * height; xy++)
        walls_or_floors[xy] = true;

      // width * y + x
      // Create some rooms
      // constexpr int room_min_size = 6;
      // constexpr int room_max_size = 10;
      // constexpr int max_rooms = 30;
      // for (int max_room_idx = 0; max_room_idx < max_rooms; max_room_idx++) {
      // const int room_width = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
      // const int room_height = static_cast<int>(engine::rand_det_s(rnd.rng, room_min_size, room_max_size));
      // const int x = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.width - room_width - 1));
      // const int y = static_cast<int>(engine::rand_det_s(rnd.rng, 0, d.height - room_height - 1));
      // Room room;
      // room.x1 = x;
      // room.y1 = y;
      // room.x2 = room.x1 + room_width;
      // room.y2 = room.y1 + room_height;
      // room.w = room_width;
      // room.h = room_height;
      // Check if the room overlaps with any of the rooms
      // const auto it =
      //   std::find_if(d.rooms.begin(), d.rooms.linend(), [&room](const Room& other) { return rooms_overlap(room, other);
      //   });
      // if (it != d.rooms.linend())
      //   continue; // overlap; skip this room
      // create_room(editor, game, d, room, ++room_index);
      // dig out a tunnel between this room and the previous one
      // bool starting_room = max_room_idx == 0;
      // if (!starting_room) {
      //   auto r0_center = room_center(d.rooms[d.rooms.size() - 1]);
      //   auto r1_center = room_center(room);
      //   create_tunnel(editor, game, d, r0_center.x, r0_center.y, r1_center.x, r1_center.y);
      // }

      // create_room
      // for (int x = 0; x < room.w; x++) {
      //   for (int y = 0; y < room.h; y++) {
      // EntityType et = EntityType::tile_type_floor;
      // if (x == 0 || x == room.w - 1)
      //   et = EntityType::tile_type_wall;
      // if (y == 0 || y == room.h - 1)
      //   et = EntityType::tile_type_wall;
      // if (EntityType::tile_type_floor == et) {
      //   if (d.walls_and_floors[index].linentity != entt::null)
      //     r.destroy(d.walls_and_floors[index].linentity);
      //   entt::entity e = create_dungeon_entity(editor, game, et, grid_index);
      //   StaticDungeonEntity se;
      //   se.linentity = e;
      //   se.x = grid_index.x;
      //   se.y = grid_index.y;
      //   se.room_index = room_index;
      //   d.walls_and_floors[index] = se;
      // }

      // create tunnel
      // int x1, int y1, int x2, int y2
      // move horisontally, then vertically
      // corner_x = x2;
      // corner_y = y1;
      // std::vector<std::pair<int, int>> line_0;
      // create_line(x1, y1, corner_x, corner_y, line_0);
      // create_tunnel_floor(editor, game, d, line_0);
      // b) corner_x, corner_y to x2, y2
      // std::vector<std::pair<int, int>> line_1;
      // create_line(corner_x, corner_y, x2, y2, line_1);
      // create_tunnel_floor(editor, game, d, line_1);
      // for (const auto& coord : coords) {
      // set as floor

    } // end generate dungeon
  }

  ImGui::End();
}

} // namespace game2d
