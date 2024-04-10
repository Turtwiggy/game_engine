#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_group/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"
#include "sprites/components.hpp"
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

  // clear anything thats hovered
  {
    const auto& view = r.view<HoveredComponent, SpriteComponent, TagComponent>();

    // revert colours on anything hovered
    {
      for (const auto& [e, hov, sc, tag] : view.each())
        sc.colour = get_lin_colour_by_tag(r, tag.tag);
    }

    r.remove<HoveredComponent>(view.begin(), view.end());
  }

  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);
    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    const auto [a_ent, b_ent] = collision_of_interest<CursorComponent, HoverableComponent>(r, a, b);
    r.emplace_or_replace<HoveredComponent>(b_ent);
  }

  // when the player releases the mouse...
  if (release) {

    // ... clear anything thats already selected (unless shift is held)
    if (!shift_held) {
      const auto& view_selected = r.view<SelectedComponent, SpriteComponent, TagComponent>();
      for (const auto& [selected_e, selected_c, sc, tag] : view_selected.each())
        sc.colour = get_lin_colour_by_tag(r, tag.tag);
      r.remove<SelectedComponent>(view_selected.begin(), view_selected.end());
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
}
};