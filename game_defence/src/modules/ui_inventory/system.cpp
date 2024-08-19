#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

#include "imgui.h"
#include <magic_enum.hpp>

#include <fmt/core.h>

namespace game2d {

void
update_item_parent(entt::registry& r, const entt::entity item, const entt::entity parent)
{
  auto& item_c = r.get<ItemComponent>(item);

  // move out of old parent
  r.get<InventorySlotComponent>(item_c.parent_slot).item_e = entt::null;

  // set item to new parent
  r.get<ItemComponent>(item).parent_slot = parent;

  // set parent to new child
  r.get<InventorySlotComponent>(parent).item_e = item;
};

const auto item_slot_accepting_item = [](entt::registry& r, const entt::entity item_slot) {
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

const auto display_empty_item = [](const InventorySlotType& type, const ImVec2& size) {
  ImGui::Button("...", size);

  // tooltip
  const std::string label = std::string(magic_enum::enum_name(type));
  ImGui::SetItemTooltip("%s", label.c_str());

  // if (ImGui::BeginItemTooltip()) {
  //   ImGui::Text("I am a fancy tooltip");
  //   static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
  //   ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
  //   ImGui::Text("Sin(time) = %f", sinf((float)ImGui::GetTime()));
  //   ImGui::EndTooltip();
  // }
};

const auto display_item = [](entt::registry& r, entt::entity slot_e, entt::entity item_e, const ImVec2& size) {
  const auto item_type = r.get<ItemComponent>(item_e).type;
  const auto item_type_str = std::string(magic_enum::enum_name(item_type));

  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));

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

  const uint32_t eid = static_cast<uint32_t>(slot_e);
  const std::string label = "inv-button##" + std::to_string(eid);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::ImageButton(label.c_str(), im_id, size, tl, br);
  ImGui::PopStyleVar();
  ImGui::SetItemTooltip("%s", item_type_str.c_str());
};

void
display_inventory_slot(entt::registry& r,
                       const entt::entity inventory_slot_e,
                       const ImVec2& button_size,
                       const ImVec2 pos = { 0, 0 })
{
  if (pos.x != 0)
    ImGui::SetCursorPosX(pos.x);
  if (pos.y != 0)
    ImGui::SetCursorPosY(pos.y);

  const auto& slot_c = r.get<InventorySlotComponent>(inventory_slot_e);
  const auto item_e = slot_c.item_e;

  if (item_e != entt::null) {
    display_item(r, inventory_slot_e, item_e, button_size);
  }

  if (item_e == entt::null) {
    display_empty_item(slot_c.type, button_size);
  }

  // item = dragdrop source
  if (item_e != entt::null)
    item_slot_has_item(item_e);

  // no item = dragdrop target
  if (item_e == entt::null)
    item_slot_accepting_item(r, inventory_slot_e);
};

void
update_ui_inventory_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

#if defined(_DEBUG)
  static bool first_time = false;
  if (first_time) {
    first_time = false;
    destroy_first_and_create<ShowInventoryRequest>(r);
  }
#endif
  toggle_inventory_display(r);

  const auto& view = r.view<PlayerComponent, DefaultBody, DefaultInventory>();
  for (const auto& [e, player_c, body_c, inv_c] : view.each()) {

    if (auto* init = r.try_get<InitBodyAndInventory>(e)) {
      r.remove<InitBodyAndInventory>(e);

      const auto create_item = [](entt::registry& r, const entt::entity slot_e, const ItemType& item_type) -> entt::entity {
        ItemComponent item;
        item.parent_slot = slot_e;
        item.type = item_type;
        const auto item_e = create_empty<ItemComponent>(r, item);
        update_item_parent(r, item_e, slot_e);
        return item_e;
      };

      // init body
      r.get<InventorySlotComponent>(body_c.body[0]).item_e = create_item(r, body_c.body[0], ItemType::head_protection);
      r.get<InventorySlotComponent>(body_c.body[1]).item_e = create_item(r, body_c.body[1], ItemType::core_protection);
      r.get<InventorySlotComponent>(body_c.body[2]).item_e = create_item(r, body_c.body[2], ItemType::arm_protection);
      r.get<InventorySlotComponent>(body_c.body[3]).item_e = create_item(r, body_c.body[3], ItemType::arm_protection);
      r.get<InventorySlotComponent>(body_c.body[4]).item_e = create_item(r, body_c.body[4], ItemType::leg_protection);
      r.get<InventorySlotComponent>(body_c.body[5]).item_e = create_item(r, body_c.body[5], ItemType::leg_protection);
      r.get<InventorySlotComponent>(body_c.body[6]).item_e = create_item(r, body_c.body[6], ItemType::scrap_shotgun);
      r.get<InventorySlotComponent>(body_c.body[7]).item_e = create_item(r, body_c.body[7], ItemType::bullettype_default);

      // init inventory
      r.get<InventorySlotComponent>(inv_c.inv[inv_c.inv.size() - 1]).item_e =
        create_item(r, inv_c.inv[inv_c.inv.size() - 1], ItemType::scrap);
      r.get<InventorySlotComponent>(inv_c.inv[inv_c.inv.size() - 2]).item_e =
        create_item(r, inv_c.inv[inv_c.inv.size() - 2], ItemType::bullettype_bouncy);
    }

    // Four cases to handle here.
    // UI: yes. Instance: yes. all good.
    // UI: no. Instance: no. all good.
    // UI: yes. Instance: no. spawn.
    // UI: no. Instance: yes. destroy.
    const auto slots = get_slots(r, e, InventorySlotType::gun);
    const bool ui_has_gun = r.get<InventorySlotComponent>(slots[0]).item_e != entt::null;
    auto* has_weapon = r.try_get<HasWeaponComponent>(e);
    const bool gun_has_instance = has_weapon && has_weapon->instance != entt::null;

    // spawn
    if (ui_has_gun && !gun_has_instance) {
      // create weapon
      // TODO: create the correct weapon spawn from inventory item type, not just weapon_shotgun
      const auto weapon = create_gameplay(r, EntityType::weapon_shotgun, get_position(r, e));
      const auto& player_team_c = r.get<TeamComponent>(e);
      r.emplace_or_replace<TeamComponent>(weapon, player_team_c.team);
      // setup weapon
      auto& weapon_parent = r.get<HasParentComponent>(weapon);
      weapon_parent.parent = e;
      // link player&weapon
      HasWeaponComponent has_weapon;
      has_weapon.instance = weapon;
      r.emplace<HasWeaponComponent>(e, has_weapon);
    }

    // destroy
    if (!ui_has_gun && gun_has_instance) {
      auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
      dead.dead.emplace(has_weapon->instance);
      r.remove<HasWeaponComponent>(e);
    }

    // Ammo. Two cases.
    // UI: must have gun in gun slot
    // Instance: must exist.
    // Ammo: must exist in slot.
    // Add/remove abletoshoot component.
    const auto ammo_slots = get_slots(r, e, InventorySlotType::bullet);
    const auto ui_ammo_slot = r.get<InventorySlotComponent>(slots[0]);
    const bool ui_has_ammo = ui_ammo_slot.item_e != entt::null;
    if (ui_has_gun && gun_has_instance)
      r.emplace_or_replace<AbleToShoot>(has_weapon->instance);

    // Ammo. Set bullet type.
    if (ui_has_gun && gun_has_instance && ui_has_ammo) {
      auto bullet_type = EntityType::bullet_default;
      const auto& item_c = r.get<ItemComponent>(ui_ammo_slot.item_e);
      if (item_c.type == ItemType::bullettype_bouncy)
        bullet_type = EntityType::bullet_bouncy;
      if (item_c.type == ItemType::bullettype_default)
        bullet_type = EntityType::bullet_default;
      r.get<WeaponBulletTypeToSpawnComponent>(has_weapon->instance).bullet_type = bullet_type;
    }
  }

  // no request: do not show inventory
  if (get_first<ShowInventoryRequest>(r) == entt::null)
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;

  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
  {
    const float window_left_edge_padding = 20;
    const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
    const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
    const auto pos = ImVec2(viewport_pos.x + window_left_edge_padding, viewport_pos.y + viewport_size_half.y);

    // configs
    const ImVec2 button_size = ImVec2(32, 32); // make the border 48 or 64
    const ImVec2 window_0_size{ button_size.x * 8, button_size.y * 6 };
    const int inv_x = 6;
    const auto window_1_size = ImVec2{ (button_size.x * inv_x) + (button_size.x * 2), window_0_size.y };
    const auto window_1_pos = ImVec2(pos.x + window_0_size.x, viewport_pos.y + viewport_size_half.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(window_0_size, window_0_size);
    ImGui::Begin("Inventory-Equipment", NULL, flags);
    ImGui::SeparatorText("Equipment");

    const auto& body_view = r.view<PlayerComponent, DefaultBody>();
    for (const auto& [e, player_c, body_c] : body_view.each()) {
      for (size_t i = 0; i < body_c.body.size(); i++) {

        if (i > 0)
          ImGui::SameLine();

        if (r.get<InventorySlotComponent>(body_c.body[i]).type == InventorySlotType::gun)
          ImGui::NewLine();

        display_inventory_slot(r, body_c.body[i], button_size);
      }
    }

    ImGui::End();

    ImGui::SetNextWindowPos(window_1_pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(window_1_size, window_1_size);
    ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, button_size.x);
    {
      ImGui::Begin("Inventory-Backpack", NULL, flags);
      ImGui::SeparatorText("Inventory");
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