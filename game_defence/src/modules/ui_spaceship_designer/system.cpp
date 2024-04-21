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
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"

#include "imgui.h"
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

#include <set>

namespace game2d {
using namespace std::literals;

struct Line
{
  glm::ivec2 p0{ 0, 0 };
  glm::ivec2 p1{ 0, 0 };

  Line() = default;
  Line(const glm::ivec2& p0, const glm::ivec2& p1)
    : p0(p0)
    , p1(p1){};
};

struct Wall
{
  entt::entity e = entt::null;
  Line line;
};

struct Room
{
  std::vector<Wall> walls;
  AABB aabb;
};

const auto create_aabb = [](const glm::ivec2& tl, const int width, const int height) -> AABB {
  AABB aabb;
  aabb.center = { tl.x + (width / 2.0), tl.y + (height / 2.0f) };
  aabb.size = { glm::abs(width), glm::abs(height) };
  return aabb;
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
  const glm::ivec2 intersection = { intersection_x, intersection_y };

  // check the intersection lies within the line segments.
  AABB point;
  point.center = intersection;
  point.size = { 1, 1 };

  const int l0 = glm::min(x1, x2);
  const int r0 = glm::max(x1, x2);
  const int t0 = glm::min(y1, y2);
  const int b0 = glm::max(y1, y2);
  AABB line0 = create_aabb({ l0, t0 }, glm::abs(r0 - l0), glm::abs(b0 - t0));
  line0.size.x = line0.size.x == 0 ? 1 : line0.size.x;
  line0.size.y = line0.size.y == 0 ? 1 : line0.size.y;

  const int l = glm::min(x3, x4);
  const int r = glm::max(x3, x4);
  const int t = glm::min(y3, y4);
  const int b = glm::max(y3, y4);
  AABB line1 = create_aabb({ l, t }, glm::abs(r - l), glm::abs(b - t));
  line1.size.x = line1.size.x == 0 ? 1 : line1.size.x;
  line1.size.y = line1.size.y == 0 ? 1 : line1.size.y;

  const bool coll_a = collide(point, line0);
  const bool coll_b = collide(point, line1);
  ImGui::Text("coll_a %i coll_b %i", coll_a, coll_b);
  if (!coll_a || !coll_b) {
    return std::nullopt; // no intersection
  }
  return intersection;
}

std::optional<glm::ivec2>
line_intersection(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3)
{
  return line_intersection(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
};

std::optional<glm::ivec2>
line_intersection(const Line& a, const Line& b)
{
  return line_intersection(a.p0, a.p1, b.p0, b.p1);
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

  static int spaceship_index_to_edit = 0;
  static SpaceshipComponent default_spaceship = {};
  static SpaceshipComponent& spaceship_to_edit = default_spaceship;
  const auto& view = r.view<SpaceshipComponent>();

  if (view.size() == 0) // ensure one spaceship
    const auto spaceship_e = create_gameplay(r, EntityType::actor_spaceship);

  // Display spaceship info
  //
  for (int i = 0; const auto& [e, spaceship] : view.each()) {
    ImGui::Separator();
    ImGui::Text("Spaceship %i", i);
    if (i == spaceship_index_to_edit)
      spaceship_to_edit = spaceship;

    // for (const auto& p : spaceship.points) {
    //   const auto& p_t = r.get<TransformComponent>(p);
    //   ImGui::Text("Point: %i, %i", p_t.position.x, p_t.position.y);
    // }

    break; // only first for moment
    i++;
  }

  // Alignment for spaceship point
  static int l_align = 0;
  static int r_align = 0;
  static int u_align = 0;
  static int d_align = 0;
  // clang-format off
  if (get_key_down(input, toggle_l_align)) {l_align = l_align == 0 ? 1 : 0;}
  if (get_key_down(input, toggle_r_align)) {r_align = r_align == 0 ? 1 : 0;}
  if (get_key_down(input, toggle_u_align)) {u_align = u_align == 0 ? 1 : 0;}
  if (get_key_down(input, toggle_d_align)) {d_align = d_align == 0 ? 1 : 0;}
  // clang-format on
  ImGui::Text("l_align %i", l_align);
  ImGui::Text("r_align %i", r_align);
  ImGui::Text("u_align %i", u_align);
  ImGui::Text("d_align %i", d_align);

  // Clamp mouse position to grid
  //
  glm::ivec2 mouse_pos = engine::grid::grid_space_to_world_space(
    engine::grid::world_space_to_grid_space(input_mouse_pos, grid_snap_size), grid_snap_size);
  mouse_pos += glm::vec2(grid_snap_size / 2.0f, grid_snap_size / 2.0f); // center
  mouse_pos += (l_align == 1) ? glm::ivec2(-grid_snap_size / 2.0f, 0.0f) : glm::ivec2{ 0, 0 };
  mouse_pos += (r_align == 1) ? glm::ivec2(grid_snap_size / 2.0f, 0.0f) : glm::ivec2{ 0, 0 };
  mouse_pos += (u_align == 1) ? glm::ivec2(0.0f, -grid_snap_size / 2.0f) : glm::ivec2{ 0, 0 };
  mouse_pos += (d_align == 1) ? glm::ivec2(0.0f, grid_snap_size / 2.0f) : glm::ivec2{ 0, 0 };

  // Free the mouse from the grid
  if (get_key_held(input, free_from_grid_key))
    mouse_pos = input_mouse_pos;

  // Work out if any of the spaceship-points are selected.
  // If they are, change the colour of the point to the hovered colour.
  //
  entt::entity selected_0 = entt::null;
  entt::entity selected_1 = entt::null;
  int selected_size = 0;
  const auto& selected_view = r.view<const SelectedComponent, const SpaceshipPointComponent, SpriteComponent>();
  for (int i = 0; const auto& [selected_e, selected_c, selected_point, sc] : selected_view.each()) {

    const auto se = static_cast<uint32_t>(selected_e);
    ImGui::Text("Selected: %i", se);

    if (i == 0)
      selected_0 = selected_e;
    if (i == 1)
      selected_1 = selected_e;

    // update colour
    const auto colour = get_lin_colour_by_tag(r, "hovered");
    sc.colour = colour;

    i++;
    selected_size = i;
  }
  const bool both_selected_valid = selected_0 != entt::null && selected_1 != entt::null;

  // Point Editor
  //
  {
    const int grid_object_size = 5;
    //
    // Create a spaceship point
    //
    if (get_key_down(input, create_point_key)) {
      const auto wall_e = create_gameplay(r, EntityType::solid_spaceship_point);
      set_position(r, wall_e, { mouse_pos });
      set_size(r, wall_e, { grid_object_size, grid_object_size });
      // spaceship_to_edit.points.push_back(wall_e);
    }
    //
    // delete selected points
    //
    if (get_key_down(input, delete_points_key)) {
      for (const auto& [selected_e, selected_c, selected_point, sc] : selected_view.each())
        dead.dead.emplace(selected_e);
    }
    //
    // join selected points
    //
    if (selected_size == 2 && get_key_down(input, join_points_key) && both_selected_valid) {
      const auto& a_t = r.get<TransformComponent>(selected_0);
      const auto& b_t = r.get<TransformComponent>(selected_1);
      const entt::entity line_e = create_gameplay(r, EntityType::solid_wall);
      const LineInfo line_info = generate_line(a_t.position, b_t.position, 1);
      set_position_and_size_with_line(r, line_e, line_info);
      // finalize... add line to spaceship!
    }
  }

  // Create a door along two points
  //
  {
    if (selected_size == 2 && get_key_down(input, create_door_key) && both_selected_valid) {
      const auto& a_t = r.get<TransformComponent>(selected_0);
      const auto& b_t = r.get<TransformComponent>(selected_1);
      const entt::entity door_e = create_gameplay(r, EntityType::solid_spaceship_door);

      const glm::vec3 diff = (b_t.position - a_t.position);
      const glm::vec3 midpoint = diff / 2.0f;
      const glm::ivec2 door_position = { a_t.position.x + midpoint.x, a_t.position.y + midpoint.y };
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
      const glm::ivec2 pressureplate_0_pos =
        door_position + glm::ivec2(nrm_a * glm::vec2{ dst, dst }) - dst_away_from_center;
      const glm::ivec2 pressureplate_1_pos =
        door_position + glm::ivec2(nrm_a * glm::vec2{ dst, dst }) + dst_away_from_center;
      // negative side
      const glm::ivec2 pressureplate_2_pos =
        door_position + glm::ivec2(nrm_b * glm::vec2{ dst, dst }) - dst_away_from_center;
      const glm::ivec2 pressureplate_3_pos =
        door_position + glm::ivec2(nrm_b * glm::vec2{ dst, dst }) + dst_away_from_center;

      set_position(r, pressureplate_e_0, pressureplate_0_pos);
      set_position(r, pressureplate_e_1, pressureplate_1_pos);
      set_position(r, pressureplate_e_2, pressureplate_2_pos);
      set_position(r, pressureplate_e_3, pressureplate_3_pos);

      set_size(r, pressureplate_e_0, { size, size });
      set_size(r, pressureplate_e_1, { size, size });
      set_size(r, pressureplate_e_2, { size, size });
      set_size(r, pressureplate_e_3, { size, size });
    }
  }

  const auto create_aabb_from_mouse = [&mouse_pos](const glm::ivec2& p) -> AABB {
    // width & height can be negative
    const int width = mouse_pos.x - p.x;
    const int height = mouse_pos.y - p.y;
    return create_aabb(p, width, height);
  };

  // Drag To Create Rooms
  //
  static std::vector<Room> rooms;
  {
    const int line_width = 1;
    static std::vector<Wall> walls;
    static std::optional<glm::ivec2> press_location = std::nullopt;
    static bool room_collision = false;
    ImGui::Text("Room Collision: %i", room_collision);

    if (get_key_down(input, drag_create_room_key) && press_location == std::nullopt) {
      // set_keydown_state();
      press_location = mouse_pos;
      room_collision = false;
      walls.clear();

      // create 4 walls
      for (int i = 0; i < 4; i++) {
        Wall w;
        w.e = create_gameplay(r, EntityType::solid_wall);
        set_position(r, w.e, press_location.value());
        set_size(r, w.e, { 16, 16 });
        walls.push_back(w);
      }
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

      walls[0].line = { tl, tr };
      walls[1].line = { tr, br };
      walls[2].line = { br, bl };
      walls[3].line = { bl, tl };
      for (const auto& w : walls)
        set_position_and_size_with_line(r, w.e, generate_line(w.line.p0, w.line.p1, line_width));

      // Check if room collides with another existing room
      const AABB r1 = create_aabb_from_mouse(press_location.value());
      for (const auto& room : rooms)
        room_collision |= collide(r1, room.aabb);
    }
    if (get_key_up(input, drag_create_room_key)) {

      // Clean up old room
      // if(collision_with_other_rooms
      // create_doors_at_collision_wall
      if (room_collision) {
        for (const auto& wall : walls)
          dead.dead.emplace(wall.e);
      }

      // finalize room
      if (!room_collision) {
        Room r;
        r.walls = std::move(walls);
        r.aabb = create_aabb_from_mouse(press_location.value());
        rooms.push_back(r);
      }

      // set_keyup_state();
      walls.clear();
      press_location = std::nullopt;
      room_collision = false;
    }
  }

  // Debug the hovered grid position
  //
  AABB aabb_gridbox;
  std::vector<Line> aabb_gridbox_to_lines;
  {
    static std::vector<entt::entity> wall_debug_lines;
    if (wall_debug_lines.size() == 0) {
      for (int i = 0; i < 4; i++) {
        auto e = create_gameplay(r, EntityType::empty_with_physics);
        set_position(r, e, mouse_pos);
        set_size(r, e, { 16, 16 });
        wall_debug_lines.push_back(e);
      }
    }
    const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, grid_snap_size);
    ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);
    ImGui::Text("cursor_pos: %i %i", input_mouse_pos.x, input_mouse_pos.y);
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
    for (int i = 0; i < 4; i++) {
      const auto e = wall_debug_lines[i];
      const auto line = aabb_gridbox_to_lines[i];
      set_position_and_size_with_line(r, e, generate_line(line.p0, line.p1, 1));
    }
  }

  // Delete Wall Segments
  //
  // if (get_key_down(input, delete_segment_key)) {
  {
    std::set<std::tuple<int, int>> coll_points;
    static std::vector<entt::entity> debug_coll_points;

    // Get all of the lines that cross over this grid_section
    for (const auto& room : rooms) {
      for (const Wall& l : room.walls) { // treat each wall as a line

        // Create an AABB for the wall (line).
        //
        const glm::ivec2 line_tl{ l.line.p0.x <= l.line.p1.x ? l.line.p0.x : l.line.p1.x,
                                  l.line.p0.y <= l.line.p1.y ? l.line.p0.y : l.line.p1.y };
        // Fix: if a line is directly horizontal, height would be 0, meaning no collision
        const int start_width = glm::abs(l.line.p1.x - l.line.p0.x);
        const int start_height = glm::abs(l.line.p1.y - l.line.p0.y);
        const int width = (start_width == 0 && start_height != 0) ? 1 : start_width;
        const int height = (start_height == 0 && start_width != 0) ? 1 : start_height;
        const auto aabb_wall_line = create_aabb(line_tl, width, height);

        // Check if the two collide.
        if (!collide(aabb_wall_line, aabb_gridbox))
          continue;

        //
        // work out if it's a end-line-segment or middle-line-segment case.
        //

        // With a Line-Line Intersection Algorithm,
        // Figure out the intersection point of:
        // the grid-box boundary & the wall-section.
        //

        for (const auto& aabb_line : aabb_gridbox_to_lines) {
          ImGui::Separator();

          const Line& l0 = aabb_line; // grid box, made of 2 points
          const Line& l1 = l.line;    // wall line, made of 2 points
          // ImGui::Text("L0: %i %i, %i %i", l0.p0.x, l0.p0.y, l0.p1.x, l0.p1.y);
          // ImGui::Text("L1: %i %i, %i %i", l1.p0.x, l1.p0.y, l1.p1.x, l1.p1.y);

          const auto maybe_intersection_point = line_intersection(l0, l1);
          if (maybe_intersection_point.has_value()) {
            glm::ivec2 i = maybe_intersection_point.value();
            ImGui::Text("Intersection! %i %i", i.x, i.y);
            coll_points.emplace(std::make_tuple(i.x, i.y));
          }
        }

        // work out collision points between the line and the aabb.
        // walls.push_back(l);
      }
    }

    // Debug points of intersection
    for (int i = 0; const std::tuple<int, int>& coll : coll_points) {
      const auto& [pos_x, pos_y] = coll;

      // create a new debug point
      if (debug_coll_points.size() == i)
        debug_coll_points.push_back(create_gameplay(r, EntityType::empty_with_transform));

      const auto& e = debug_coll_points[i];
      set_position(r, e, { pos_x, pos_y });
      set_size(r, e, { 6, 6 });

      i++;
    }
    ImGui::Text("Intersections: %i", coll_points.size());
  }

  // for (const auto& w : walls)
  //   ImGui::Text("Wall of interest selected");

  // Misc
  //
  {
    // HACK: Spawn a Player
    //
    if (get_mouse_rmb_press()) {
      const auto player = create_gameplay(r, EntityType::actor_player);
      set_position(r, player, mouse_pos);
    }

    // Debug Rooms
    //
    for (const auto& room : rooms) {
      ImGui::Text("RoomExists!");
      for (const Wall& w : room.walls) {
        ImGui::SameLine();
        ImGui::Text("Wall");
      }
    }

    // Generate dungeon base
    //
    if (get_key_down(input, generate_dungeon_key)) {
      // Delete old Dungeon
      for (Room& room : rooms) {
        for (Wall& wall : room.walls)
          dead.dead.emplace(wall.e);
        room.walls.clear();
      }
      rooms.clear();

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
      //   std::find_if(d.rooms.begin(), d.rooms.end(), [&room](const Room& other) { return rooms_overlap(room, other); });
      // if (it != d.rooms.end())
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
      //   if (d.walls_and_floors[index].entity != entt::null)
      //     r.destroy(d.walls_and_floors[index].entity);
      //   entt::entity e = create_dungeon_entity(editor, game, et, grid_index);
      //   StaticDungeonEntity se;
      //   se.entity = e;
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
