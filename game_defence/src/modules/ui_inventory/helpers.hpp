#pragma once

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
update_item_parent(entt::registry& r, const entt::entity item, const entt::entity parent);

entt::entity
create_inv_scrap(entt::registry& r, const entt::entity slot_e);

} // namespace game2d