#pragma once

#include "actors/bags/armour.hpp"
#include "actors/bags/bullets.hpp"
#include "modules/ui_inventory/components.hpp"

#include "entt/entt.hpp"
#include "imgui.h"

#include <string>
#include <utility>

namespace game2d {

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite);

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

// items

entt::entity
create_inv_scrap(entt::registry& r, const entt::entity slot_e);

entt::entity
create_inv_armour(entt::registry& r, const entt::entity slot_e, const ArmourType& type);

entt::entity
create_inv_breachcharge(entt::registry& r, const entt::entity slot_e);

entt::entity
create_inv_shotgun(entt::registry& r, const entt::entity slot_e);

entt::entity
create_inv_bullets(entt::registry& r, const entt::entity slot_e, const BulletType& type);

//

} // namespace game2d