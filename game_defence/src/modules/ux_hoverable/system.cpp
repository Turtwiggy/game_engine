#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_cursor/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ux_hoverable(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  auto& hi = get_first_component<SINGLE_HoverInfo>(r);

  // warning: only works with mouse
  const bool release = get_mouse_lmb_release();

  // warning: physics updated every fixedupdate
  // this system is update

  hi.hovered.clear();

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
      hi.hovered.emplace(b);
    else if (b_cursor && a_hoverable)
      hi.hovered.emplace(a);
  }

  // move hovering to selected
  if (release) {
    hi.selected.clear();
    std::move(hi.hovered.begin(), hi.hovered.end(), std::back_inserter(hi.selected));
  }

  // Debug this system
  ImGui::Begin("Hovering");
  for (const auto& e : hi.hovered)
    ImGui::Text("Hovering something");
  for (const auto& e : hi.selected)
    ImGui::Text("Selected something");
  ImGui::End();
}

};