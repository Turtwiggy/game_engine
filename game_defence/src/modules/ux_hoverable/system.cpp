#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"

#include <SDL_scancode.h>

namespace game2d {

void
update_ux_hoverable(entt::registry& r)
{
  // warning: physics updated every fixedupdate
  // this system is update
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  // warning: only works with mouse
  const bool release = get_mouse_lmb_release();
  const bool shift_held = get_key_held(input, SDL_SCANCODE_LSHIFT);

  // Clear anything thats hovered
  {
    const auto& view = r.view<HoveredComponent>();
    r.remove<HoveredComponent>(view.begin(), view.end());
  }

  // Work out what's being hovered this tick
  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);
    const auto [a_ent, b_ent] = collision_of_interest<CursorComponent, HoverableComponent>(r, a, b);
    r.emplace_or_replace<HoveredComponent>(b_ent);
  }

  // when the player releases the mouse...
  if (release) {

    // ... clear anything thats already selected (unless shift is held)
    if (!shift_held) {
      const auto& view = r.view<SelectedComponent>();
      r.remove<SelectedComponent>(view.begin(), view.end());
    }

    // ... move hovering to selected
    const auto& view = r.view<HoveredComponent>();
    for (const auto& [e, hovered] : view.each()) {
      //
      // For the moment, limit selected to groupos
      r.emplace_or_replace<SelectedComponent>(e);

      // limit to players
      // if (team.team == AvailableTeams::player)
      // r.emplace_or_replace<SelectedComponent>(e);
    }
  }
};

} // namespace game2d