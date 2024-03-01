#include "modules/items_pickup/system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <iostream>
#include <optional>
#include <vector>

#include "imgui.h"

namespace game2d {

// for (const auto& coll : physics.collision_stay) {
//   const auto a = static_cast<entt::entity>(coll.ent_id_0);
//   const auto b = static_cast<entt::entity>(coll.ent_id_1);
// }

void
update_intent_pickup_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  const auto& items_view =
    r.view<const ItemComponent, const AABB, const EntityTypeComponent>(entt::exclude<WaitForInitComponent>);
  const auto& players_view = r.view<PlayerComponent>(entt::exclude<WaitForInitComponent>);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  //
  // System: check collisions between player and dropped items
  // List them in a ui menu
  //

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  static bool show_pickup_menu = true;
  ImGui::Begin("Pickup Items Menu", &show_pickup_menu, flags);

  for (const auto& [item_e, item_c, item_aabb, item_type] : items_view.each()) {
    const auto eid = static_cast<uint32_t>(item_e);
    ImGui::PushID(eid);

    ImGui::Text("Item");
    ImGui::SameLine();

    if (ImGui::Button("Pickup")) {
      for (const auto& [player_e, player_c] : players_view.each()) {
        //
        // item implementations? Probably should not be here.
        // Change if items get more than one type of item.
        //
        if (item_type.type == EntityType::actor_pickup_xp)
          player_c.picked_up_xp += 1;

        // Give the item to the player
        r.remove<AABB>(item_e);
        r.remove<TransformComponent>(item_e);
        r.emplace<HasParentComponent>(item_e, player_e);
        break; // the first player
      }
    }
    ImGui::PopID();
  }

  ImGui::End();
}

} // namespace game2d
