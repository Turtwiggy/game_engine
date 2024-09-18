#include "helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_scene_main_menu/helpers.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

namespace game2d {

void
toggle_inventory_display(entt::registry& r)
{
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_E)) {
    // not showing => showing
    if (get_first<ShowInventoryRequest>(r) == entt::null)
      destroy_first_and_create<ShowInventoryRequest>(r);
    // showing => not showing
    else
      destroy_first<ShowInventoryRequest>(r);
  }
};

void
update_item_parent(entt::registry& r, const entt::entity item, const entt::entity parent_slot)
{
  auto& item_c = r.get<ItemComponent>(item);

  // move out of old parent
  r.get<InventorySlotComponent>(item_c.parent_slot).item_e = entt::null;

  // set item to new parent
  r.get<ItemComponent>(item).parent_slot = parent_slot;

  // set parent to new child
  r.get<InventorySlotComponent>(parent_slot).item_e = item;
};

void
handle_dragdrop_target(entt::registry& r, const entt::entity payload_e, const entt::entity slot_e)
{
  auto& slot_c = r.get<InventorySlotComponent>(slot_e);
  const bool slot_is_full = slot_c.item_e != entt::null;
  const bool item_exists = payload_e != entt::null;

  // 4 cases:
  // yes item, full slot
  // no item, full slot
  // yes item, free slot
  // no item, free slot

  // yes item, free slot
  if (item_exists && !slot_is_full)
    update_item_parent(r, payload_e, slot_e);

  // yes item, full slot (i.e. inventory swapping items)
  if (item_exists && slot_is_full) {
    auto item_0_e = payload_e;
    auto item_1_e = slot_c.item_e;
    auto& item_0_c = r.get<ItemComponent>(item_0_e);
    auto& item_1_c = r.get<ItemComponent>(item_1_e);
    const auto slot_0_e = item_0_c.parent_slot;
    const auto slot_1_e = item_1_c.parent_slot;

    // update child <=> parent
    item_0_c.parent_slot = slot_1_e;
    item_1_c.parent_slot = slot_0_e;

    // update parent <=> child
    r.get<InventorySlotComponent>(item_0_c.parent_slot).item_e = item_0_e;
    r.get<InventorySlotComponent>(item_1_c.parent_slot).item_e = item_1_e;
  }
};

void
become_dragdrop_target(entt::registry& r, const entt::entity slot_e)
{
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ITEM_E_PAYLOAD")) {
      IM_ASSERT(payload->DataSize == sizeof(uint32_t));
      const auto payload_eid = *(const uint32_t*)payload->Data;
      const auto payload_e = static_cast<entt::entity>(payload_eid);

      handle_dragdrop_target(r, payload_e, slot_e);
    }
    ImGui::EndDragDropTarget();
  }
};

void
become_dragdrop_source(const entt::entity item_e)
{
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
    ImGui::SetDragDropPayload("ITEM_E_PAYLOAD", &item_e, sizeof(uint32_t));
    ImGui::EndDragDropSource();
  }
};

void
display_empty_item(entt::registry& r, entt::entity slot_e, const InventorySlotType& type, const ImVec2& size)
{
  auto& ui = get_first_component<SINGLE_UIInventoryState>(r);

  const uint32_t eid = static_cast<uint32_t>(slot_e);
  const std::string label = "inv-button##" + std::to_string(eid);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::Button(" ", size);
  ImGui::PopStyleVar();

  // sounds
  play_sound_if_hovered(r, ui.hovered_buttons, label);

  // tooltip
  const std::string label_tooltip = std::string(magic_enum::enum_name(type));
  ImGui::SetItemTooltip("%s", label_tooltip.c_str());

  // if (ImGui::BeginItemTooltip()) {
  //   ImGui::Text("I am a fancy tooltip");
  //   static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
  //   ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
  //   ImGui::Text("Sin(time) = %f", sinf((float)ImGui::GetTime()));
  //   ImGui::EndTooltip();
  // }
};

void
display_item(entt::registry& r, entt::entity slot_e, entt::entity item_e, const ImVec2& size)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  const auto& item_tag = r.get<TagComponent>(item_e);
  auto& ui = get_first_component<SINGLE_UIInventoryState>(r);

  ImVec2 tl{ 0.0f, 0.0f };
  ImVec2 br{ 1.0f, 1.0f };

  const auto& item_c = r.get<ItemComponent>(item_e);
  const auto result = convert_sprite_to_uv(r, item_c.display_icon);
  std::tie(tl, br) = result;

  const uint32_t eid = static_cast<uint32_t>(slot_e);
  const std::string label = "inv-button##" + std::to_string(eid);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::ImageButton(label.c_str(), im_id, size, tl, br);
  ImGui::PopStyleVar();

  // sounds
  play_sound_if_hovered(r, ui.hovered_buttons, label);

  // tooltip
  ImGui::SetItemTooltip("%s", item_c.display_name.c_str());
};

void
display_inventory_slot(entt::registry& r, const entt::entity inventory_slot_e, const ImVec2& button_size, const ImVec2 pos)
{
  if (pos.x != 0)
    ImGui::SetCursorPosX(pos.x);
  if (pos.y != 0)
    ImGui::SetCursorPosY(pos.y);

  const auto& slot_c = r.get<InventorySlotComponent>(inventory_slot_e);
  const auto item_e = slot_c.item_e;

  if (item_e != entt::null)
    display_item(r, inventory_slot_e, item_e, button_size);

  if (item_e == entt::null)
    display_empty_item(r, inventory_slot_e, slot_c.type, button_size);

  if (item_e != entt::null)
    become_dragdrop_source(item_e);

  // any slot is a target, even if full
  become_dragdrop_target(r, inventory_slot_e);
};

void
update_initialize_inventory(entt::registry& r, entt::entity e)
{
  auto& body_c = r.get<DefaultBody>(e);
  auto& inv_c = r.get<DefaultInventory>(e);

  if (auto* init = r.try_get<InitBodyAndInventory>(e)) {
    r.remove<InitBodyAndInventory>(e);

    // init body
    r.get<InventorySlotComponent>(body_c.body[0]).item_e = spawn_item(r, "scrap_helmet");
    r.get<InventorySlotComponent>(body_c.body[1]).item_e = spawn_item(r, "scrap_core");
    r.get<InventorySlotComponent>(body_c.body[2]).item_e = spawn_item(r, "scrap_gloves");
    r.get<InventorySlotComponent>(body_c.body[3]).item_e = spawn_item(r, "scrap_gloves");
    r.get<InventorySlotComponent>(body_c.body[4]).item_e = spawn_item(r, "scrap_legs");
    r.get<InventorySlotComponent>(body_c.body[5]).item_e = spawn_item(r, "scrap_legs");
    r.get<InventorySlotComponent>(body_c.body[6]).item_e = spawn_item(r, "breach charge");

    // initial items in your inventory
    const auto inv_1_e = inv_c.inv[inv_c.inv.size() - 1];
    const auto inv_2_e = inv_c.inv[inv_c.inv.size() - 2];
    const auto inv_3_e = inv_c.inv[inv_c.inv.size() - 3];
    const auto inv_4_e = inv_c.inv[inv_c.inv.size() - 4];
    const auto inv_5_e = inv_c.inv[inv_c.inv.size() - 5];
    r.get<InventorySlotComponent>(inv_1_e).item_e = spawn_item(r, "scrap");
    r.get<InventorySlotComponent>(inv_2_e).item_e = spawn_item(r, "shotgun");
    r.get<InventorySlotComponent>(inv_3_e).item_e = spawn_item(r, "bullet_default");
    r.get<InventorySlotComponent>(inv_4_e).item_e = spawn_item(r, "bullet_bouncy");
    r.get<InventorySlotComponent>(inv_5_e).item_e = spawn_item(r, "breach charge");
  }
};

} // namespace game2d