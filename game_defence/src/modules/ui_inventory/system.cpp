#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

#include "imgui.h"
#include <magic_enum.hpp>

#include <fmt/core.h>
#include <ranges>

namespace game2d {

void
update_item_parent(entt::registry& r, const entt::entity& item, const entt::entity& parent)
{
  auto& item_c = r.get<ItemComponent>(item);

  // move out of old parent
  r.get<InventorySlotComponent>(item_c.parent_slot).child_item = entt::null;

  // set item to new parent
  r.get<ItemComponent>(item).parent_slot = parent;

  // set parent to new child
  r.get<InventorySlotComponent>(parent).child_item = item;
};

const auto item_slot_accepting_item = [](entt::registry& r, const entt::entity& item_slot) {
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ITEM_E_PAYLOAD")) {
      IM_ASSERT(payload->DataSize == sizeof(uint32_t));
      const auto payload_eid = *(const uint32_t*)payload->Data;
      const auto payload_e = static_cast<entt::entity>(payload_eid);
      update_item_parent(r, payload_e, item_slot);
    }
    ImGui::EndDragDropTarget();
  }
};

const auto item_slot_has_item = [](const entt::entity item_e) {
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
    ImGui::SetDragDropPayload("ITEM_E_PAYLOAD", &item_e, sizeof(uint32_t));
    ImGui::EndDragDropSource();
  }
};

void
display_inventory_slot(entt::registry& r,
                       const entt::entity& inventory_slot_e,
                       const ImVec2& button_size,
                       ImVec2 pos = { 0, 0 })
{
  auto& slot = r.get<InventorySlotComponent>(inventory_slot_e);

  if (pos.x != 0)
    ImGui::SetCursorPosX(pos.x);
  if (pos.y != 0)
    ImGui::SetCursorPosY(pos.y);

  if (slot.child_item != entt::null) {
    const auto item_type = r.get<ItemComponent>(slot.child_item).type;
    const auto item_type_str = std::string(magic_enum::enum_name(item_type));

    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
    const auto id = (ImTextureID)tex_id;

    ImVec2 tl{ 0.0f, 0.0f };
    ImVec2 br{ 1.0f, 1.0f };

    if (auto it{ item_to_sprite_map.find(item_type) }; it != std::end(item_to_sprite_map)) {
      const auto& [key, val]{ *it };
      if (val != "") {
        const auto result = convert_sprite_to_uv(r, val);
        std::tie(tl, br) = result;
      } else {
        // item_to_spritemap set but it's set as ""
        const auto result = convert_sprite_to_uv(r, "TEXT_?");
        std::tie(tl, br) = result;
      }
    }
    // item_to_spritemap not set
    else {
      const auto result = convert_sprite_to_uv(r, "TEXT_?");
      std::tie(tl, br) = result;
    }

    const uint32_t eid = static_cast<uint32_t>(inventory_slot_e);
    const std::string label = "inv-button##" + std::to_string(eid);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::ImageButton(label.c_str(), id, button_size, tl, br);
    ImGui::PopStyleVar();
    ImGui::SetItemTooltip("%s", item_type_str.c_str());

  } else {
    ImGui::Button("...", button_size);
    const std::string label = std::string(magic_enum::enum_name(slot.type));
    ImGui::SetItemTooltip("%s", label.c_str());

    // if (ImGui::BeginItemTooltip()) {
    //   ImGui::Text("I am a fancy tooltip");
    //   static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
    //   ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
    //   ImGui::Text("Sin(time) = %f", sinf((float)ImGui::GetTime()));
    //   ImGui::EndTooltip();
    // }
  }

  // slot contains an item. can be come a dragdrop source
  if (slot.child_item != entt::null)
    item_slot_has_item(slot.child_item);

  // slot does not contain an item. can become a dragdrop target
  if (slot.child_item == entt::null)
    item_slot_accepting_item(r, inventory_slot_e);
};

void
update_ui_inventory_system(entt::registry& r)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

#if defined(_DEBUG)
  static bool first_time = true;
  if (first_time) {
    first_time = false;
    destroy_first_and_create<ShowInventoryRequest>(r);
  }
#endif

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

  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
  {
    const float window_left_edge_padding = 20;
    const auto viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
    const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
    const auto pos = ImVec2(viewport_pos.x + window_left_edge_padding, viewport_pos.y + viewport_size_half.y);

    // configs
    const int inv_x = 6;
    const int inv_y = 5;
    const ImVec2 button_size = ImVec2(40, 40);
    const ImVec2 gun_button_size = button_size;
    const ImVec2 window_0_size{ 300, 300 };
    const ImVec2 window_1_size{ (button_size.x * inv_x) + (button_size.x * 2), window_0_size.y };

    // window 1
    const float center_x = window_0_size.x / 2.0f - button_size.x / 2.0f;
    const float right_x = window_0_size.x;
    const float base_y = button_size.y / 2.0f;
    const int padding_x = 10;
    const int padding_y = 0;
    const auto window_1_pos = ImVec2(pos.x + window_0_size.x, viewport_pos.y + viewport_size_half.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(window_0_size, window_0_size);
    ImGui::Begin("Inventory-Body", NULL, flags);

    const auto create_body_slot = [&r](const InventorySlotType& type) -> entt::entity {
      return create_empty<InventorySlotComponent>(r, InventorySlotComponent{ type });
    };
    static auto head_eid = create_body_slot(InventorySlotType::head);
    static auto chest_eid = create_body_slot(InventorySlotType::chest);
    static auto l_arm_eid = create_body_slot(InventorySlotType::l_arm);
    static auto r_arm_eid = create_body_slot(InventorySlotType::r_arm);
    static auto legs_eid = create_body_slot(InventorySlotType::legs);
    static auto feet_eid = create_body_slot(InventorySlotType::feet);
    static auto gun_0_eid = create_body_slot(InventorySlotType::gun);
    static auto gun_1_eid = create_body_slot(InventorySlotType::gun);

    const auto create_item = [&r](const entt::entity& slot_e, const ItemType& item_type) -> entt::entity {
      ItemComponent item;
      item.parent_slot = slot_e;
      item.type = item_type;
      const auto item_e = create_empty<ItemComponent>(r, item);
      update_item_parent(r, item_e, slot_e);
      return item_e;
    };

    // give player default items?
    static auto scrap_item_head = create_item(head_eid, ItemType::scrap_helmet);
    static auto scrap_item_chest = create_item(chest_eid, ItemType::scrap_chestpiece);
    static auto scrap_item_l_arm = create_item(l_arm_eid, ItemType::scrap_arm_bracer);
    static auto scrap_item_r_arm = create_item(r_arm_eid, ItemType::scrap_arm_bracer);
    static auto scrap_item_legs = create_item(legs_eid, ItemType::scrap_legs);
    static auto scrap_item_feet = create_item(feet_eid, ItemType::scrap_boots);
    static auto scrap_item_gun = create_item(gun_0_eid, ItemType::scrap_shotgun);

    {
      float y = base_y;
      display_inventory_slot(r, head_eid, button_size, { center_x, 0 });

      y += button_size.y + padding_y;
      display_inventory_slot(r, l_arm_eid, button_size, { center_x - button_size.x - padding_x, 0 });
      ImGui::SameLine();
      display_inventory_slot(r, chest_eid, button_size, { center_x, 0 });
      ImGui::SameLine();
      display_inventory_slot(r, r_arm_eid, button_size, { center_x + button_size.x + padding_x, 0 });

      y += button_size.y + padding_y;
      display_inventory_slot(r, legs_eid, button_size, { center_x, 0 });
      ImGui::SameLine();
      display_inventory_slot(r, gun_0_eid, gun_button_size, { right_x - gun_button_size.x - padding_x, 0 });

      y += button_size.y + padding_y;
      display_inventory_slot(r, feet_eid, button_size, { center_x, 0 });
      ImGui::SameLine();
      display_inventory_slot(r, gun_1_eid, gun_button_size, { right_x - gun_button_size.x - padding_x, 0 });
    }

    ImGui::End();

    //
    // create the backpack inventory
    // n.b. should occur after creating the body
    //

    // Show a 6x5 grid in the window

    static bool created_inventory = false;
    if (!created_inventory) {

      for (int i = 0; i < inv_x * inv_y; i++) {
        const auto slot_e = create_empty<InventorySlotComponent>(r, InventorySlotComponent{ InventorySlotType::backpack });
        if (i == (inv_x * inv_y) - 1)
          create_item(slot_e, ItemType::scrap);
      }
      created_inventory = true;
    }

    ImGui::SetNextWindowPos(window_1_pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(window_1_size, window_1_size);
    ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, button_size.x);
    {
      ImGui::Begin("Inventory-Backpack", NULL, flags);
      {
        const int columns = inv_x;
        ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame;

        ImGui::BeginTable("backpack", columns, table_flags);

        for (const auto& [e, slot] : r.view<InventorySlotComponent>().each()) {
          if (slot.type != InventorySlotType::backpack)
            continue;
          const auto eid = static_cast<uint32_t>(e);
          ImGui::PushID(eid);

          ImGui::TableNextColumn();
          display_inventory_slot(r, e, button_size);

          ImGui::PopID();
        }

        ImGui::EndTable();
      }
      ImGui::End();
    }
    ImGui::PopStyleVar();
  }
  ImGui::PopStyleVar();
}

} // namespace game2d