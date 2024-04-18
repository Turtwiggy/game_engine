#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
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
#include "renderer/transform.hpp"
#include "sprites/components.hpp"

#include "imgui.h"
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

namespace game2d {
using namespace std::literals;

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::ivec2& input_mouse_pos)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto create_point_key = SDL_SCANCODE_C;
  const auto create_linked_point_key = SDL_SCANCODE_R;
  const auto join_points_key = SDL_SCANCODE_F;
  const auto snap_to_grid_key = SDL_SCANCODE_LSHIFT;
  const auto delete_points_key = SDL_SCANCODE_DELETE;
  const auto create_door_key = SDL_SCANCODE_G;
  const auto toggle_l_align = SDL_SCANCODE_LEFT;
  const auto toggle_r_align = SDL_SCANCODE_RIGHT;
  const auto toggle_u_align = SDL_SCANCODE_UP;
  const auto toggle_d_align = SDL_SCANCODE_DOWN;

  static int grid_snap_size = 50;
  static int grid_object_size = 5;
  static float line_width = 5.0f;

  imgui_draw_int("edit_gridsize", grid_snap_size);
  imgui_draw_int("edit_grid_object_size", grid_object_size);
  auto& grid_effect = get_first_component<Effect_GridComponent>(r);
  grid_effect.gridsize = grid_snap_size;

  ImGui::Begin("SpaceshipDesigner");
  // ImGui::Text("Cursor: %i %i", mouse_pos.x, mouse_pos.y);
  ImGui::Text("create_point_key: %s", SDL_GetScancodeName(create_point_key));
  ImGui::Text("create_linked_point_key: %s", SDL_GetScancodeName(create_linked_point_key));
  ImGui::Text("join_point_key: %s", SDL_GetScancodeName(join_points_key));
  ImGui::Text("snap_to_grid_key: %s", SDL_GetScancodeName(snap_to_grid_key));
  ImGui::Text("delete_points_key: %s", SDL_GetScancodeName(delete_points_key));
  ImGui::Text("create_door_key: %s", SDL_GetScancodeName(create_door_key));

  static int spaceship_index_to_edit = 0;
  static SpaceshipComponent default_spaceship = {};
  static SpaceshipComponent& spaceship_to_edit = default_spaceship;
  const auto& view = r.view<SpaceshipComponent>();

  if (view.size() == 0) // ensure one spaceship
    create_gameplay(r, EntityType::actor_spaceship);

  // Display spaceship info
  //
  for (int i = 0; const auto& [e, spaceship] : view.each()) {
    ImGui::Separator();
    ImGui::Text("Spaceship %i", i);
    if (i == spaceship_index_to_edit)
      spaceship_to_edit = spaceship;

    for (const auto& p : spaceship.points) {
      const auto& p_t = r.get<TransformComponent>(p);
      ImGui::Text("Point: %i, %i", p_t.position.x, p_t.position.y);
    }

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

  // Clamp mouse positiion to grid
  //
  glm::ivec2 mouse_pos = input_mouse_pos;
  if (get_key_held(input, snap_to_grid_key)) {
    const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, grid_snap_size);
    mouse_pos = engine::grid::grid_space_to_world_space(grid_pos, grid_snap_size);

    // center
    mouse_pos += glm::vec2(grid_snap_size / 2.0f, grid_snap_size / 2.0f);

    // Left or Right
    if (l_align == 1)
      mouse_pos += glm::vec2(-grid_snap_size / 2.0f, 0.0f);
    else if (r_align == 1)
      mouse_pos += glm::vec2(grid_snap_size / 2.0f, 0.0f);

    // Up or Down
    if (u_align == 1)
      mouse_pos += glm::vec2(0.0f, -grid_snap_size / 2.0f);
    else if (d_align == 1)
      mouse_pos += glm::vec2(0.0f, grid_snap_size / 2.0f);
  }

  // Create a spaceship point
  //
  if (get_key_down(input, create_point_key)) {
    const auto wall_e = create_gameplay(r, EntityType::solid_spaceship_point);
    set_position(r, wall_e, { mouse_pos });
    set_size(r, wall_e, grid_object_size);
    spaceship_to_edit.points.push_back(wall_e);
  }

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

  // join selected points
  //
  const bool both_selected_valid = selected_0 != entt::null && selected_1 != entt::null;
  if (selected_size == 2 && get_key_down(input, join_points_key) && both_selected_valid) {
    const auto& a_t = r.get<TransformComponent>(selected_0);
    const auto& b_t = r.get<TransformComponent>(selected_1);
    const entt::entity line_e = create_gameplay(r, EntityType::solid_wall);
    const LineInfo line_info = generate_line(a_t.position, b_t.position, line_width);

    set_position_with_line(r, line_e, line_info);

    // finalize... add line to spaceship!
  }

  // delete selected points
  //
  if (get_key_down(input, delete_points_key)) {
    for (const auto& [selected_e, selected_c, selected_point, sc] : selected_view.each())
      dead.dead.emplace(selected_e);
  }

  // Create a door along two points
  //
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
    const glm::ivec2 pressureplate_0_pos = door_position + glm::ivec2(nrm_a * glm::vec2{ dst, dst }) - dst_away_from_center;
    const glm::ivec2 pressureplate_1_pos = door_position + glm::ivec2(nrm_a * glm::vec2{ dst, dst }) + dst_away_from_center;
    // negative side
    const glm::ivec2 pressureplate_2_pos = door_position + glm::ivec2(nrm_b * glm::vec2{ dst, dst }) - dst_away_from_center;
    const glm::ivec2 pressureplate_3_pos = door_position + glm::ivec2(nrm_b * glm::vec2{ dst, dst }) + dst_away_from_center;

    set_position(r, pressureplate_e_0, pressureplate_0_pos);
    set_position(r, pressureplate_e_1, pressureplate_1_pos);
    set_position(r, pressureplate_e_2, pressureplate_2_pos);
    set_position(r, pressureplate_e_3, pressureplate_3_pos);

    set_size(r, pressureplate_e_0, { size, size });
    set_size(r, pressureplate_e_1, { size, size });
    set_size(r, pressureplate_e_2, { size, size });
    set_size(r, pressureplate_e_3, { size, size });
  }

  ImGui::End();
}

} // namespace game2d

// Create Linked Point on to a selected point
//
// static entt::entity hover_hint_entity = entt::null;
// if (selected_view.size_hint() == 1 && r.valid(selected_0)) {
//   // create hover-hint entity
//   if (hover_hint_entity == entt::null)
//     hover_hint_entity = create_gameplay(r, EntityType::empty_with_transform);
//   const auto pos0 = get_position(r, selected_0);
//   const auto size0 = get_size(r, selected_0);
//   const float d2_to_corner = size0.x * size0.x + size0.y * size0.y;
//   const float d_to_corner = glm::sqrt(d2_to_corner);
//   const auto dir_raw = glm::vec2{ mouse_pos.x, mouse_pos.y } - pos0;
//   const auto dir_nrm = engine::normalize_safe(dir_raw);
//   const bool x_bigger_than_y = glm::abs(dir_nrm.x) > glm::abs(dir_nrm.y);
//   // Show a hover-hint for the point to place
//   glm::vec2 pos1{ pos0.x, pos0.y };
//   if (dir_nrm.x >= 0 && x_bigger_than_y) {
//     pos1.x += size0.x;
//     pos1.y += dir_nrm.y * d_to_corner;
//   }
//   if (dir_nrm.x < 0 && x_bigger_than_y) {
//     pos1.x -= size0.x;
//     pos1.y += dir_nrm.y * d_to_corner;
//   }
//   if (dir_nrm.y >= 0 && !x_bigger_than_y) {
//     pos1.x += dir_nrm.x * d_to_corner;
//     pos1.y += size0.y;
//   }
//   if (dir_nrm.y < 0 && !x_bigger_than_y) {
//     pos1.x += dir_nrm.x * d_to_corner;
//     pos1.y -= size0.y;
//   }
//   set_position(r, hover_hint_entity, pos1);
//   if (get_key_down(input, create_linked_point_key)) {
//     // finalize a new point at hover hint position
//     const auto new_point = create_gameplay(r, EntityType::solid_spaceship_point);
//     set_position(r, new_point, pos1);

//     r.remove<SelectedComponent>(selected_0);
//     r.emplace<SelectedComponent>(new_point);
//   }
// }
// // cleanup hover-hint
// if (selected_view.size_hint() == 0 && hover_hint_entity != entt::null) {
//   if (r.valid(hover_hint_entity))
//     r.destroy(hover_hint_entity);
//   hover_hint_entity = entt::null;
// }