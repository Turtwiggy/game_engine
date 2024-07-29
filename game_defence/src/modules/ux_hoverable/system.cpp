#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/grid/components.hpp"

#include "imgui.h"

#include <SDL_scancode.h>

namespace game2d {

void
update_ux_hoverable(entt::registry& r, const glm::ivec2& mouse_pos)
{
  // warning: only works with mouse
  // const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  // const bool release = get_mouse_lmb_release();
  // const bool shift_held = get_key_held(input, SDL_SCANCODE_LSHIFT);

  ImGui::Begin("DebugHover");

  // Clear anything thats hovered
  {
    const auto& view = r.view<HoveredComponent>();
    ImGui::Text("Hovered before: %i", view.size());
    r.remove<HoveredComponent>(view.begin(), view.end());
  }

  // Work out what's being hovered (via map)
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto grid_pos = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
  const auto grid_idx = engine::grid::grid_position_to_clamped_index(grid_pos, map_c.xmax, map_c.ymax);
  ImGui::Text("Grid Idx: %i", grid_idx);

  const auto& grid_es = map_c.map[grid_idx];
  ImGui::Text("Ents: %i", grid_es.size());

  for (const auto& map_e : grid_es) {
    if (const auto* has_health = r.try_get<HealthComponent>(map_e))
      r.emplace_or_replace<HoveredComponent>(map_e);
  }

  {
    const auto& view = r.view<HoveredComponent>();
    ImGui::Text("Hovered after: %i", view.size());
  }

  // Work out what's being hovered this tick (via physics)
  // const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  // for (const auto& coll : physics.collision_stay) {
  //   const auto a = static_cast<entt::entity>(coll.ent_id_0);
  //   const auto b = static_cast<entt::entity>(coll.ent_id_1);
  //   const auto [a_ent, b_ent] = collision_of_interest<CursorComponent, HoverableComponent>(r, a, b);
  //   r.emplace_or_replace<HoveredComponent>(b_ent);
  // }

  // when the player releases the mouse...
  // if (release) {

  //   // ... clear anything thats already selected (unless shift is held)
  //   if (!shift_held) {
  //     const auto& view = r.view<SelectedComponent>();
  //     r.remove<SelectedComponent>(view.begin(), view.end());
  //   }

  //   // ... move hovering to selected
  //   const auto& view = r.view<HoveredComponent, TeamComponent>();
  //   for (const auto& [e, hovered, team] : view.each()) {
  //     //
  //     // For the moment, limit selected to groupo
  //     // r.emplace_or_replace<SelectedComponent>(e);

  //     // limit to players
  //     if (team.team == AvailableTeams::player)
  //       r.emplace_or_replace<SelectedComponent>(e);
  //   }
  // }

  ImGui::End();
};

} // namespace game2d