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
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
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

  const SDL_Scancode create_point_key = SDL_SCANCODE_C;
  const auto* create_point_key_name = SDL_GetScancodeName(create_point_key);

  const SDL_Scancode create_linked_point_key = SDL_SCANCODE_R;
  const auto* create_linked_point_key_name = SDL_GetScancodeName(create_linked_point_key);

  const SDL_Scancode join_points_key = SDL_SCANCODE_F;
  const auto* join_point_key_name = SDL_GetScancodeName(join_points_key);

  const auto snap_to_grid_key = SDL_SCANCODE_LSHIFT;
  const auto* snap_to_grid_key_name = SDL_GetScancodeName(snap_to_grid_key);

  const auto delete_points_key = SDL_SCANCODE_DELETE;
  const auto* delete_points_key_name = SDL_GetScancodeName(delete_points_key);

  static float grid_snap_size = 32.0f;
  static float line_width = 10.0f;

  bool clamped = false;
  glm::ivec2 mouse_pos = input_mouse_pos;
  if (get_key_held(input, snap_to_grid_key)) {
    const auto grid_pos = engine::grid::world_space_to_grid_space(mouse_pos, grid_snap_size);
    mouse_pos = engine::grid::grid_space_to_world_space(grid_pos, grid_snap_size);
    mouse_pos += glm::vec2(grid_snap_size / 2.0f, grid_snap_size / 2.0f);
    clamped = true;
  }

  ImGui::Begin("SpaceshipDesigner");
  ImGui::Text("Cursor: %i %i, clamped: %i", mouse_pos.x, mouse_pos.y, clamped);
  ImGui::Text("create_point_key_name: %s", create_point_key_name);
  ImGui::Text("create_linked_point_key_name: %s", create_linked_point_key_name);
  ImGui::Text("join_point_key_name: %s", join_point_key_name);
  ImGui::Text("snap_to_grid_key_name: %s", snap_to_grid_key_name);
  ImGui::Text("delete_points_key_name: %s", delete_points_key_name);
  imgui_draw_float("Line Width"s, line_width);

  if (ImGui::Button("Create##spaceship"))
    create_gameplay(r, EntityType::actor_spaceship);

  static int spaceship_index_to_edit = 0;
  static SpaceshipComponent default_spaceship = {};
  static SpaceshipComponent& spaceship_to_edit = default_spaceship;
  const auto& view = r.view<SpaceshipComponent>();

  if (view.size() == 0) // ensure one spaceship
    create_gameplay(r, EntityType::actor_spaceship);

  for (int i = 0; const auto& [e, spaceship] : view.each()) {
    ImGui::Text("Spaceship");

    if (i == spaceship_index_to_edit)
      spaceship_to_edit = spaceship;

    for (const auto& p : spaceship.points) {
      const auto& p_t = r.get<TransformComponent>(p);
      ImGui::Text("Point: %i, %i", p_t.position.x, p_t.position.y);
    }

    i++;
  }

  if (get_key_down(input, create_point_key)) {
    const auto wall_e = create_gameplay(r, EntityType::solid_spaceship_point);
    set_position(r, wall_e, { mouse_pos });
    spaceship_to_edit.points.push_back(wall_e);
  }

  entt::entity selected_0 = entt::null;
  entt::entity selected_1 = entt::null;
  const auto& selected_view = r.view<SelectedComponent, SpaceshipPointComponent, SpriteComponent>();
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
  }

  // join 2 selected points
  //
  const int num_seleected_points = 2;
  if (selected_view.size_hint() == num_seleected_points && get_key_up(input, join_points_key)) {
    const auto& a_t = r.get<TransformComponent>(selected_0);
    const auto& b_t = r.get<TransformComponent>(selected_1);
    const auto line_t = create_gameplay(r, EntityType::empty_with_transform);
    const auto line = generate_line(a_t.position, b_t.position, line_width);
    set_transform_with_line(r.get<TransformComponent>(line_t), line);

    // finalize... add line to spaceship!
    //   // spaceship_to_edit.points.push_back(release_pos);
    //   // spaceship_to_edit.edges.push_back({index_of_press_pos, index_of_release_pos});
  }

  // Create Linked Point on to a selected point
  //
  static entt::entity hover_hint_entity = entt::null;
  if (selected_view.size_hint() == 1) {
    // create hover-hint entity
    if (hover_hint_entity == entt::null)
      hover_hint_entity = create_gameplay(r, EntityType::empty_with_transform);

    const auto pos0 = get_position(r, selected_0);
    const auto dir_raw = glm::vec2{ mouse_pos.x, mouse_pos.y } - pos0;
    const auto dir_nrm = engine::normalize_safe(dir_raw);

    // Show a hover-hint for the point to place
    // TODO: continue this
    glm::vec2 pos1{ pos0.x, pos0.y };
    if (dir_nrm.x >= 0) {
      pos1.x += 32;
      pos1.y += dir_nrm.y * 10;
    }

    // if (dir_nrm.x < 0)
    //   pos1.x -= 32;
    // if (dir_nrm.y >= 0)
    //   pos1.y += 32;
    // if (dir_nrm.y < 0)
    //   pos1.y -= 32;

    set_position(r, hover_hint_entity, pos1);

    if (get_key_down(input, create_linked_point_key)) {
      // finalize a new point at hover hint position
      const auto new_point = create_gameplay(r, EntityType::solid_spaceship_point);
      set_position(r, new_point, pos1);

      r.remove<SelectedComponent>(selected_0);
      r.emplace<SelectedComponent>(new_point);
    }
  }
  // cleanup hover-hint
  if (selected_view.size_hint() == 0 && hover_hint_entity != entt::null) {
    if (r.valid(hover_hint_entity))
      r.destroy(hover_hint_entity);
    hover_hint_entity = entt::null;
  }

  // delete selected points
  //
  if (get_key_down(input, delete_points_key)) {
    for (const auto& [selected_e, selected_c, selected_point, sc] : selected_view.each())
      dead.dead.emplace(selected_e);
  }

  ImGui::End();
}

} // namespace game2d