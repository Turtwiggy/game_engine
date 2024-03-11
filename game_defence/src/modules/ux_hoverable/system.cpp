#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_group/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ux_hoverable(entt::registry& r)
{
  // warning: physics updated every fixedupdate
  // this system is update
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // warning: only works with mouse
  const bool release = get_mouse_lmb_release();

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

    const auto* a_cursor = r.try_get<CursorComponent>(a);
    const auto* b_cursor = r.try_get<CursorComponent>(b);
    const auto* a_hoverable = r.try_get<HoverableComponent>(a);
    const auto* b_hoverable = r.try_get<HoverableComponent>(b);

    if (a_cursor && b_hoverable)
      r.emplace_or_replace<HoveredComponent>(b);
    else if (b_cursor && a_hoverable)
      r.emplace_or_replace<HoveredComponent>(a);
  }

  // when the player releases the mouse...
  if (release) {

    // ... clear anything thats already selected
    {
      const auto& view_selected = r.view<SelectedComponent>();
      r.remove<SelectedComponent>(view_selected.begin(), view_selected.end());
    }

    // ... move hovering to selected
    const auto& view = r.view<HoveredComponent, GroupComponent>();
    for (const auto& [e, hovered, team] : view.each()) {
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