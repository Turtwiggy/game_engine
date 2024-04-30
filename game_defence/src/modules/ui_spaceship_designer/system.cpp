#include "system.hpp"

#include "actors.hpp"
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/entity_pool.hpp"
#include "helpers/line.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "modules/system_spaceship_door/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"

#include "imgui.h"
#include "renderer/transform.hpp"
#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <string>

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
  // order matters for these
  glm::ivec2 p0{ 0, 0 };
  glm::ivec2 p1{ 0, 0 };
  entt::entity parent_room = entt::null;

  std::vector<glm::ivec2> intersections; // storage for when this wall is being intersected
  std::vector<entt::entity> debug_intersections;
};

struct Room
{
  bool placeholder = true;
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
  // ImGui::Text("Point. Center: %i %i. Size: %i %i", point.center.x, point.center.y, point.size.x, point.size.y);

  const int l0 = glm::min(x1, x2);
  const int r0 = glm::max(x1, x2);
  const int t0 = glm::min(y1, y2);
  const int b0 = glm::max(y1, y2);
  AABB line0 = create_aabb({ l0, t0 }, glm::abs(r0 - l0), glm::abs(b0 - t0));
  line0.size.x = line0.size.x == 0 ? 1 : line0.size.x;
  line0.size.y = line0.size.y == 0 ? 1 : line0.size.y;
  // ImGui::Text("line0. Center: %i %i. Size: %i %i", line0.center.x, line0.center.y, line0.size.x, line0.size.y);

  const int l = glm::min(x3, x4);
  const int r = glm::max(x3, x4);
  const int t = glm::min(y3, y4);
  const int b = glm::max(y3, y4);
  AABB line1 = create_aabb({ l, t }, glm::abs(r - l), glm::abs(b - t));
  line1.size.x = line1.size.x == 0 ? 1 : line1.size.x;
  line1.size.y = line1.size.y == 0 ? 1 : line1.size.y;
  // ImGui::Text("line1. Center: %i %i. Size: %i %i", line1.center.x, line1.center.y, line1.size.x, line1.size.y);

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
};

entt::entity
create_wall(entt::registry& r, const glm::ivec2 pos, const entt::entity& parent_room)
{
  const auto wall_e = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_e, pos);
  set_size(r, wall_e, { 16, 16 });

  Wall w;
  w.parent_room = parent_room;
  r.emplace<Wall>(wall_e, w);

  return wall_e;
};

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::ivec2& input_mouse_pos, const float dt)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto debug_points_pool_e = get_first<EntityPool>(r);
  if (debug_points_pool_e == entt::null)
    r.emplace<EntityPool>(create_gameplay(r, EntityType::empty_no_transform));
  auto& debug_points_pool_c = get_first_component<EntityPool>(r);

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
      const auto w0 = create_wall(r, press_location.value(), room_to_create);
      const auto w1 = create_wall(r, press_location.value(), room_to_create);
      const auto w2 = create_wall(r, press_location.value(), room_to_create);
      const auto w3 = create_wall(r, press_location.value(), room_to_create);
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
  std::vector<std::tuple<glm::ivec2, glm::ivec2>> clipped_pairs;

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
        new_line.parent_room = wall_c.parent_room;
        r.emplace<Wall>(new_wall_e, new_line);

        clipped_pairs.push_back(std::make_tuple(i0, i1));
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
  for (const auto& [i0, i1] : clipped_pairs) {
    // glm::ivec2 more_south_intersection_point;
    // if (intersection_points[0].y > intersection_points[1].y)
    //   more_south_intersection_point = intersection_points[0];
    // else
    //   more_south_intersection_point = intersection_points[1];
    // // horizontal or vertical
    // const glm::vec2 dir = intersection_points[1] - intersection_points[0];
    // const auto horizontal = dir.x > dir.y;

    // glm::ivec2 new_door_point_a;
    // glm::ivec2 new_door_point_b;
    // if (horizontal) {
    //   new_door_point_a = more_south_intersection_point;
    //   new_door_point_a.x -= 25;
    //   new_door_point_b = more_south_intersection_point;
    //   new_door_point_b.x += 25;
    // } else {
    //   new_door_point_a = more_south_intersection_point;
    //   new_door_point_a.y -= 25;
    //   new_door_point_b = more_south_intersection_point;
    //   new_door_point_b.y += 25;
    // }
    // create_door_along_two_points(r, new_door_point_a, new_door_point_b);

    create_door_along_two_points(r, i0, i1);
  }
  clipped_pairs.clear();

  // Render all walls
  const auto& wall_view = r.view<Wall>();
  for (const auto& [wall_e, wall_c] : wall_view.each())
    set_position_and_size_with_line(r, wall_e, generate_line(wall_c.p0, wall_c.p1, 1));

  // Render points of intersection
  {
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

  // TODO: Debug Rooms
  // const auto& rooms_view = r.view<Room>();
  // for (const auto& [room_e, room_c] : rooms_view.each())
  //   ImGui::Text("RoomExists! Walls: %i", room_c.walls.size());

  // Misc
  //
  {
    // HACK: Spawn a Player
    if (get_mouse_rmb_press()) {
      const auto player = create_gameplay(r, EntityType::actor_player);
      set_position(r, player, mouse_pos);
    }

    // Generate dungeon base
    if (get_key_down(input, generate_dungeon_key)) {
      // // Delete old Dungeon
      // for (const auto& [room_e, room_c] : rooms_view.each()) {
      //   for (const entt::entity& w : room_c.walls)
      //     dead.dead.emplace(w);
      //   dead.dead.emplace(room_e);
      // }

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

  // If you put your mouse over a new room,
  // (todo) play some music,
  // and display the room name.
  static std::optional<std::string> room_name = std::nullopt;
  for (const auto& coll : physics.collision_enter) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    const auto [a_ent, b_ent] = collision_of_interest<PlayerComponent, Room>(r, a, b);
    if (a_ent == entt::null)
      continue;
    if (b_ent == entt::null)
      continue;

    // Display some fun text
    // This just displays the entt id atm
    room_name = "Room "s + std::to_string(static_cast<uint32_t>(b_ent));
  }
  if (room_name.has_value())
    ImGui::Text("Last Entered Room: %s", room_name.value().c_str());
  else
    ImGui::Text("No room entered");
  ImGui::End();

  //
  // Room Name Hover System
  //
  const float time_to_display_room_name = 6.0f;
  static float time_displaying_room_name = 0.0f;

  if (room_name.has_value())
    time_displaying_room_name += dt;
  if (time_displaying_room_name > time_to_display_room_name) {
    room_name = std::nullopt;
    time_displaying_room_name = 0.0f;
  }

  bool show_room_ui = time_displaying_room_name <= time_to_display_room_name && time_displaying_room_name != 0.0f;
  if (show_room_ui) {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
    const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);

    // text size
    std::string label = "N/A";
    if (room_name.has_value())
      label = room_name.value();
    ImGuiStyle& style = ImGui::GetStyle();
    const float alignment = 0.5f;
    const float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;

    // window size
    const float forced_padding = size / 2.0;
    const float w = size + forced_padding;
    const float h = w * (9 / 16.0f) / 2.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, h), ImVec2(w, h));

    // position
    const float center_x = viewport_pos.x + viewport_size_half.x;
    const float bottom_y = viewport_pos.y + ri.viewport_size_current.y - h;
    const auto pos = ImVec2(center_x, bottom_y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;

    ImGui::Begin("Last Room Entered", &show_room_ui, flags);

    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - size) * alignment;
    if (off > 0.0f)
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    ImGui::Text("%s", label.c_str());

    ImGui::End();
  }
}

} // namespace game2d
