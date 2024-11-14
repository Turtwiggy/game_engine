#include "ui_inventory_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_helpers.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_scene_main_menu/helpers.hpp"

#include "imgui.h"
#include "magic_enum.hpp"
#include "ui_inventory_components.hpp"

namespace game2d {

entt::entity
spawn_inv_item(entt::registry& r, std::vector<entt::entity>& v, int idx, std::string key)
{
  const auto e = spawn_item(r, key);

  r.remove<TransformComponent>(e);
  r.remove<SpriteComponent>(e);
  if (auto* timer_c = r.try_get<EntityTimedLifecycle>(e))
    r.remove<EntityTimedLifecycle>(e);
  if (auto* callback_c = r.try_get<OnDeathCallback>(e))
    r.remove<OnDeathCallback>(e);
  if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {
    auto& physics_c = get_first_component<SINGLE_Physics>(r);
    physics_c.world->DestroyBody(pb->body);
    r.remove<PhysicsBodyComponent>(e);
  }

  // set child's parent
  r.get<UI_ItemComponent>(e).parent_slot = v[idx];

  // set parent's child
  r.get<InventorySlotComponent>(v[idx]).item_e = e;

  return e;
};

entt::entity
get_slot_type(entt::registry& r, const std::vector<entt::entity>& slots, const InventorySlotType& type)
{
  entt::entity ui_gun_slot_e = entt::null;
  for (const entt::entity slot_e : slots) {
    auto& slot_c = r.get<InventorySlotComponent>(slot_e);
    if (slot_c.type == InventorySlotType::weapon)
      ui_gun_slot_e = slot_e;
  }
  return ui_gun_slot_e;
};

void
toggle_inventory_display(entt::registry& r)
{
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_I)) {
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
  auto& item_c = r.get<UI_ItemComponent>(item);

  // move out of old parent
  r.get<InventorySlotComponent>(item_c.parent_slot).item_e = entt::null;

  // set item to new parent
  r.get<UI_ItemComponent>(item).parent_slot = parent_slot;

  // set parent to new child
  r.get<InventorySlotComponent>(parent_slot).item_e = item;
};

void
handle_dragdrop_target(entt::registry& r, const entt::entity item_e, const entt::entity dst_slot_e)
{
  auto& dst_slot_c = r.get<InventorySlotComponent>(dst_slot_e);
  const bool slot_is_full = dst_slot_c.item_e != entt::null;
  const bool item_exists = item_e != entt::null;

  // 4 cases:
  // yes item, full slot
  // no item, full slot
  // yes item, free slot
  // no item, free slot

  // yes item, free slot
  if (item_exists && !slot_is_full)
    update_item_parent(r, item_e, dst_slot_e);

  // yes item, full slot (i.e. inventory swapping items)
  if (item_exists && slot_is_full) {
    auto item_0_e = item_e;
    auto item_1_e = dst_slot_c.item_e;
    auto& item_0_c = r.get<UI_ItemComponent>(item_0_e);
    auto& item_1_c = r.get<UI_ItemComponent>(item_1_e);
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
  const std::string slot_type_str = std::string(magic_enum::enum_name(type));

  float w = ImGui::GetContentRegionAvail().x;
  float h = 20;
  const float text_padding = 25.0f; // Padding for the left-aligned text

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  // a button with no text
  ImGui::Button(std::format("##{}", eid).c_str(), { w, h });
  ImGui::PopStyleVar();

  become_dragdrop_target(r, slot_e);

  // sounds
  const std::string eid_label = std::format("item-inv##{}", eid);
  play_sound_if_hovered(r, ui.hovered_buttons, eid_label);

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

  std::string text = "Empty";
  if (type != InventorySlotType::backpack)
    text = std::format("[{}] Empty", slot_type_str);

  // left-aligned text
  ImGui::SameLine();
  ImVec2 cursor_pos = ImGui::GetCursorPos();
  float text_pos_y = cursor_pos.y + (h - ImGui::GetFontSize()) * 0.5f;
  ImGui::SetCursorPosX(text_padding);
  ImGui::SetCursorPosY(text_pos_y);
  ImGui::TextUnformatted(text.c_str());
};

void
display_item(entt::registry& r, entt::entity slot_e, const InventorySlotType& type, entt::entity item_e, const ImVec2& size)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  const auto& item_tag = r.get<TagComponent>(item_e);
  const Item& item_data = r.get<Item>(item_e);
  auto& ui = get_first_component<SINGLE_UIInventoryState>(r);
  const std::string slot_type_str = std::string(magic_enum::enum_name(type));

  const float w = ImGui::GetContentRegionAvail().x;
  const float h = 20;
  const float icon_size = 20.0f;    // Size of the icon
  const float padding = 5.0f;       // Space between text and icon
  const float text_padding = 25.0f; // Padding for the left-aligned text

  ImVec2 tl{ 0.0f, 0.0f };
  ImVec2 br{ 1.0f, 1.0f };

  const auto& item_c = r.get<UI_ItemComponent>(item_e);
  const auto result = convert_sprite_to_uv(r, item_c.display_icon);
  std::tie(tl, br) = result;

  const uint32_t eid = static_cast<uint32_t>(slot_e);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  // a button with no text
  ImGui::Button(std::format("##{}", eid).c_str(), { w, h });
  become_dragdrop_source(item_e);
  become_dragdrop_target(r, slot_e);

  // sounds
  const std::string eid_label = std::format("item-inv##{}", eid);
  play_sound_if_hovered(r, ui.hovered_buttons, eid_label);

  // tooltip
  if (ImGui::BeginItemTooltip()) {

    // note: could be fun to set the colour here,
    // to represent the rarity of the item

    const auto item_name_str = item_c.display_name.c_str();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", item_name_str);

    const auto item_desc_str = item_c.display_desc.c_str();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", item_desc_str);

    // Show Damage
    const auto item_damage = get_damage_for_item(r, item_e);
    ImGui::TextColored(ImVec4(0.75, 0.3, 0.3, 1.0f), "ATK: %d", item_damage);

    // Show Defence
    if (item_data.defence.has_value()) {
      ImGui::SameLine();
      ImGui::TextColored(ImVec4(0.75, 0.3, 0.3, 1.0f), "DEF: %d", item_data.defence.value().block);
    }

    if (item_data.traits.has_value()) {
      for (int i = 0; const auto& item_trait : item_data.traits.value()) {
        const auto display_str = std::format("+{}", item_trait.key);
        ImGui::TextColored(ImVec4(0.75, 0.3, 0.3, 1.0f), "%s", display_str.c_str());
      }
    }

    auto& core_item_c = r.get<Item>(item_e);
    if (core_item_c.use.has_value())
      ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "(usable)");

    ImGui::EndTooltip();
  }

  std::string text = std::format("{}", item_tag.tag);

  // for the equipment menu, show what the equip type is
  if (type != InventorySlotType::backpack)
    text = std::format("[{}] {}", slot_type_str, item_data.display_name);

  // for the inventory menu, show where the equpment should go
  if (item_data.defence.has_value())
    if (item_data.defence.value().worn_on.has_value())
      text = std::format("[{}] {}", item_data.defence->worn_on.value(), item_data.display_name);

  // left-aligned text
  ImGui::SameLine();
  ImVec2 cursor_pos = ImGui::GetCursorPos();
  float text_pos_y = cursor_pos.y + (h - ImGui::GetFontSize()) * 0.5f;
  ImGui::SetCursorPosX(text_padding);
  ImGui::SetCursorPosY(text_pos_y);
  ImGui::TextUnformatted(text.c_str());

  // right-aligned icon
  ImGui::SameLine(w - icon_size - padding);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (h - icon_size) * 0.5f); // Center icon vertically
  ImGui::Image(im_id, { 18, 18 }, tl, br);

  ImGui::PopStyleVar();
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
    display_item(r, inventory_slot_e, slot_c.type, item_e, button_size);

  if (item_e == entt::null)
    display_empty_item(r, inventory_slot_e, slot_c.type, button_size);
};

void
update_initialize_inventory(entt::registry& r, entt::entity e)
{
  auto& body_c = r.get<DefaultBody>(e);
  auto& inv_c = r.get<DefaultInventory>(e);

  if (auto* init = r.try_get<InitBodyAndInventory>(e)) {
    r.remove<InitBodyAndInventory>(e);

    // init body with items
    // spawn_inv_item(r, body_c.body, 0, "scrap_helmet");
    // spawn_inv_item(r, body_c.body, 1, "scrap_core");
    // spawn_inv_item(r, body_c.body, 2, "scrap_gloves");
    // spawn_inv_item(r, body_c.body, 3, "scrap_gloves");
    // spawn_inv_item(r, body_c.body, 4, "scrap_legs");
    // spawn_inv_item(r, body_c.body, 5, "scrap_legs");
    spawn_inv_item(r, body_c.body, 6, "hook");

    // init inventory with items
    // note: spawn less than 6*5 items (default inventory size)
    int i = 0;
    spawn_inv_item(r, inv_c.inv, int(i++), "hook");
    spawn_inv_item(r, inv_c.inv, int(i++), "shotgun");
    spawn_inv_item(r, inv_c.inv, int(i++), "scrap_knife");
    // spawn_inv_item(r, inv_c.inv, i++, "scrap");
    // spawn_inv_item(r, inv_c.inv, i++, "bullet_default");
    // spawn_inv_item(r, inv_c.inv, i++, "bullet_bouncy");
    // spawn_inv_item(r, inv_c.inv, i++, "breach_charge");
    // spawn_inv_item(r, inv_c.inv, i++, "breach_charge");
    spawn_inv_item(r, inv_c.inv, i++, "scrap_helmet");
    spawn_inv_item(r, inv_c.inv, i++, "scrap_core");
    spawn_inv_item(r, inv_c.inv, i++, "scrap_gloves");
    spawn_inv_item(r, inv_c.inv, i++, "scrap_gloves");
    spawn_inv_item(r, inv_c.inv, i++, "scrap_legs");
    spawn_inv_item(r, inv_c.inv, i++, "scrap_legs");
  }
};

} // namespace game2d