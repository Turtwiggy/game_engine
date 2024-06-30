#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/ui_inventory/components.hpp"

#include "imgui.h"

#include <fmt/core.h>
#include <ranges>

namespace game2d {

void
check_item_parent_valid(entt::registry& r, const entt::entity& e)
{
  InventorySlot& slot = r.get<InventorySlot>(e);

  // update item to new parent
  const auto wrong_parent = [&r, &e](const auto& other) { return r.get<Item>(other).parent != e; };
  const auto [first, last] = std::ranges::remove_if(slot.items, wrong_parent);

  for (auto it = first; it != last; ++it) {
    const auto item_e = (*it);
    const auto parent_e = r.get<Item>(item_e).parent;

    // ahh! parent was never set.
    if (parent_e == entt::null) {
      fmt::println("Item parent was never set");
      continue;
    }

    // add item if it does not exist
    auto& inv = r.get<InventorySlot>(parent_e);
    if (std::ranges::find(inv.items, item_e) == inv.items.end())
      inv.items.push_back(item_e);
  }

  slot.items.erase(first, last);
};

void
update_item_parent(entt::registry& r, const entt::entity& item, const entt::entity& parent)
{
  const auto& old_parent = r.get<Item>(item).parent;

  // ahh! parent was never set.
  if (old_parent == entt::null) {
    fmt::println("Item parent was never set");
    return;
  }

  // remove the item from the old inventory slot
  auto& old_parent_slot = r.get<InventorySlot>(old_parent);
  const auto old_item = [&r, &item](const auto& other) { return other == item; };
  const auto [first, last] = std::ranges::remove_if(old_parent_slot.items, old_item);

  // add the item to the new inventory slot
  auto& inv = r.get<InventorySlot>(parent);
  if (std::ranges::find(inv.items, item) == inv.items.end())
    inv.items.push_back(item);

  // Do the erase from the old one
  old_parent_slot.items.erase(first, last);

  // Update the item parent
  r.get<Item>(item).parent = parent;
};

void
update_ui_inventory_system(entt::registry& r)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_I)) {
    // not showing => showing
    if (get_first<ShowInventoryRequest>(r) == entt::null)
      destroy_first_and_create<ShowInventoryRequest>(r);
    // showing => not showing
    else
      destroy_first<ShowInventoryRequest>(r);
  }

  // no request: do not show inventory
  if (get_first<ShowInventoryRequest>(r) == entt::null)
    return;

  ImGui::Begin("Inventory");

  static int payload_info = 0;

  // Target i.e. body
  std::string head_label = "Head " + std::to_string(payload_info);
  static entt::entity head_eid = create_empty<InventorySlot>(r, InventorySlot{ InventorySlotType::head });
  ImGui::Button(head_label.c_str());

  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("INV_PAYLOAD")) {
      IM_ASSERT(payload->DataSize == sizeof(uint32_t));
      const auto payload_eid = *(const uint32_t*)payload->Data;
      const auto payload_e = static_cast<entt::entity>(payload_eid);
      if (r.valid(payload_e)) {
        // recieve payload impl...
        update_item_parent(r, payload_e, head_eid);
      }
    }
    ImGui::EndDragDropTarget();
  }
  // if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
  //   ImGui::SetDragDropPayload("INV_PAYLOAD", &head_eid, sizeof(uint32_t));
  //   ImGui::EndDragDropSource();
  // }

  // Source i.e. inventory
  // show a 6x5 grid in the right half of the window
  const int x = 6;
  const int y = 5;

  static bool created_inventory = false;
  if (!created_inventory) {
    for (int i = 0; i < (x * y) - 1; i++) {
      const auto inventory_e = create_empty<InventorySlot>(r, InventorySlot{ InventorySlotType::backpack });
      if (i == 0) {
        const auto item_e = create_empty<Item>(r);
        auto& item_c = r.get<Item>(item_e);
        item_c.parent = inventory_e;

        auto& inventory = r.get<InventorySlot>(inventory_e);
        inventory.items.push_back(item_e);
      }
    }
    created_inventory = true;
  }

  for (int i = 0; const auto& [e, slot] : r.view<InventorySlot>().each()) {
    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    // make these slots a grid
    if (i % x == 0)
      ImGui::NewLine();
    else
      ImGui::SameLine();

    check_item_parent_valid(r, e);

    const std::string label = std::to_string(slot.items.size());
    ImGui::Button(label.c_str(), ImVec2(60, 60));

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
      if (slot.items.size() > 0) {
        const auto& first_item_e = slot.items[0];
        ImGui::SetDragDropPayload("INV_PAYLOAD", &first_item_e, sizeof(uint32_t));
      }
      ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("INV_PAYLOAD")) {
        IM_ASSERT(payload->DataSize == sizeof(uint32_t));
        const auto payload_eid = *(const uint32_t*)payload->Data;
        const auto payload_e = static_cast<entt::entity>(payload_eid);
        if (r.valid(payload_e)) {
          // recieve payload impl...
          update_item_parent(r, payload_e, e);
        }
      }
      ImGui::EndDragDropTarget();
    }

    ImGui::PopID();
    i++;
  }

  ImGui::End();
}

} // namespace game2d

// // Show like potion x1, potion x2 not potions individually
// std::map<std::string, std::vector<entt::entity>> compacted_items;
// for (const auto& [item_e, item_c, item_parent, item_tag] : items_view.each()) {
//   if (item_parent.parent != player_e)
//     continue; // not my item
//   compacted_items[item_tag.tag].push_back(item_e);
// }

// for (const auto& [tag, entity_items] : compacted_items) {

//   // assume all the compacted items have the same state
//   // this could break if potions had internal state,
//   // e.g. usages left
//   const auto& entity_item = entity_items[0];
//   const auto entity_type = r.get<EntityTypeComponent>(entity_item).type;
//   const auto entity_type_name = std::string(magic_enum::enum_name(entity_type));

//   //
//   ImGui::Text("Player has %s (%i times)", entity_type_name.c_str(), entity_items.size());
// }