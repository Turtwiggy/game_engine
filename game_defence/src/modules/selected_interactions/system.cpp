#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
update_selected_interactions_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& cursor = get_first<CursorComponent>(r);

  // warning: doesnt work with controller currently
  const bool click = get_mouse_rmb_press();
  const bool held = get_mouse_rmb_held();
  const bool release = get_mouse_rmb_release();

  std::vector<entt::entity> enemies;

  // What are we hovering?
  //
  const auto& view = r.view<HoveredComponent, TeamComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, hovered, team] : view.each()) {
    if (team.team == AvailableTeams::enemy)
      enemies.push_back(e);
  }

  // Update cursor if hovering enemies
  //
  if (enemies.size() > 0)
    set_sprite(r, cursor, "CURSOR_3");
  else
    set_sprite(r, cursor, "EMPTY");

  // move all the selected units and try to keep them in formation
  //
  if (click) {

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

    // Attack the enemies!
    //
    if (enemies.size() > 0) {

      // Tell all selected units to attack!
      const auto& selected_view = r.view<SelectedComponent>(entt::exclude<WaitForInitComponent>);
      for (const auto& [e, selected] : selected_view.each())
        r.emplace_or_replace<WantsToShoot>(e);
    }

    // Move the selected units
    //
    else {

      // set velocity to get to destination
      const auto& destination = mouse_pos;

      // Update selcted units target position
      const auto& selected_view = r.view<SelectedComponent, HasTargetPositionComponent>(entt::exclude<WaitForInitComponent>);
      for (const auto& [e, selected, target] : selected_view.each())
        target.position = destination;
    }

    //
  }

  //
}

} // namespace game2d