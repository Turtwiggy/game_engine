#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"

#include <glm/glm.hpp>

// calculate average position of selected objects
//
// const int count = hi.seleced.size();
// glm::ivec2 avg_pos{ 0, 0 };
// for (const auto& e : hi.selected) {
//   const auto& aabb = r.get<AABB>(e);
//   avg_pos += aabb.center;
// }
// if (count != 0)
//   avg_pos /= count;

namespace game2d {

void
update_selected_interactions_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& cursor = get_first<CursorComponent>(r);
  const auto& cursor_comp = r.get<CursorComponent>(cursor);
  const auto& enemies = cursor_comp.hovering_enemies;

  // warning: doesnt work with controller currently
  const bool click = get_mouse_rmb_press();
  const bool held = get_mouse_rmb_held();
  const bool release = get_mouse_rmb_release();

  static std::optional<glm::ivec2> click_position;
  static std::optional<glm::ivec2> held_position;
  if (click) {
    click_position = mouse_pos;
    r.get<TransformComponent>(cursor_comp.click_ent).position = glm::ivec3(mouse_pos.x, mouse_pos.y, 0);
  }
  if (held && enemies.size() == 0) {
    held_position = mouse_pos;
    r.get<TransformComponent>(cursor_comp.held_ent).position = glm::ivec3(mouse_pos.x, mouse_pos.y, 0);

    const auto line = generate_line(click_position.value(), held_position.value(), 2);
    auto& line_transform = r.get<TransformComponent>(cursor_comp.line_ent);
    set_transform_with_line(line_transform, line);
  }

  // move all the selected units and try to keep them in formation
  //
  if (click && enemies.size() > 0) {
    std::cout << "attack action" << std::endl;

    // Tell all selected units to attack!
    const auto& selected_view = r.view<SelectedComponent>(entt::exclude<WaitForInitComponent>);
    for (const auto& [e, selected] : selected_view.each()) {
      //
      // set target as first enemy target
      r.get_or_emplace<TargetComponent>(e).target = enemies[0];
    }
  }
  // Move the selected units
  //
  else if (release && enemies.size() == 0) {
    std::cout << "move action" << std::endl;

    // Position units along the line, facing the dir
    glm::vec2 dir = held_position.value() - click_position.value();
    if (dir.x != 0 || dir.y != 0.0f)
      dir = glm::normalize(dir);
    const glm::vec2 perp_dir = { -dir.y, dir.x };

    const auto center = click_position;
    auto destination = center.value(); // where the click was

    const auto& selected_view =
      r.view<SelectedComponent, AABB, HasTargetPositionComponent>(entt::exclude<WaitForInitComponent>);

    // Get unit info
    //
    int total_targets = 0;
    float total_length = 0;
    for (const auto& [e, selected, aabb, target] : selected_view.each()) {
      total_length += aabb.size.x;
      total_targets++;
    }
    const glm::vec2 total_targets_vec = glm::vec2(total_targets, total_targets);

    // Where is the middle unit?
    const bool is_even = total_targets % 2 == 0;
    float mid_idx = 0;
    if (total_length != 0)
      mid_idx = (total_targets + 1) / 2.0f;

    const float size_of_each_unit = total_length / total_targets;
    const glm::vec2 size{ size_of_each_unit, size_of_each_unit };

    // if is even...
    const int min_idx = glm::floor(mid_idx);
    const float half_range = size_of_each_unit * min_idx;
    const glm::vec2 min = glm::vec2(-half_range, -half_range);

    // Update selcted units target position
    //
    for (int i = 1; const auto& [e, selected, aabb, target] : selected_view.each()) {

      glm::vec2 pos = center.value();

      pos += perp_dir * ((glm::vec2(i, i) * size) - (size / 2.0f) - (size * total_targets_vec / 2.0f));

      target.position = pos;

      i++;
    }

    click_position = std::nullopt;
    held_position = std::nullopt;
  }

  // continuously shoot if a target is set
  //
  const auto& auto_shoot_view = r.view<TargetComponent>();
  for (const auto& [e, target] : auto_shoot_view.each()) {
    const auto& t = target.target;
    if (!r.valid(t) || t == entt::null) {
      r.remove<TargetComponent>(e);
      continue;
    }

    const auto& hp = r.get<HealthComponent>(t);
    if (hp.hp <= 0) {
      r.remove<TargetComponent>(e);
      continue;
    }

    // otherwise, shoot it
    r.emplace<WantsToShoot>(e);
  }
}

} // namespace game2d