#pragma once

#include "actors/bags/armour.hpp"
#include "actors/bags/bullets.hpp"
#include "modules/ui_inventory/components.hpp"

#include "entt/entt.hpp"
#include "imgui.h"

#include <string>
#include <utility>

namespace game2d {

void
toggle_inventory_display(entt::registry& r);

std::vector<entt::entity>
get_slots(entt::registry& r, const entt::entity e, const InventorySlotType& type);

void
update_item_parent(entt::registry& r, const entt::entity item, const entt::entity parent_slot);

void
display_inventory_slot(entt::registry& r,
                       const entt::entity inventory_slot_e,
                       const ImVec2& button_size,
                       const ImVec2 pos = { 0, 0 });

void
handle_dragdrop_target(entt::registry& r, const entt::entity payload_e, const entt::entity slot_e);

} // namespace game2d