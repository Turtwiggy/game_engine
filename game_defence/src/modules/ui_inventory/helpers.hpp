#pragma once

#include "entt/entt.hpp"
#include "imgui.h"

namespace game2d {

void
toggle_inventory_display(entt::registry& r);

void
update_item_parent(entt::registry& r, const entt::entity item, const entt::entity parent_slot);

void
display_inventory_slot(entt::registry& r,
                       const entt::entity inventory_slot_e,
                       const ImVec2& button_size,
                       const ImVec2 pos = { 0, 0 });

void
handle_dragdrop_target(entt::registry& r, const entt::entity payload_e, const entt::entity slot_e);

void
update_initialize_inventory(entt::registry& r, entt::entity e_player);

} // namespace game2d