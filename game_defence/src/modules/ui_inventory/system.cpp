#include "system.hpp"

#include "actors/bags/armour.hpp"
#include "actors/bags/items.hpp"
#include "actors/bags/weapons.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

#include "imgui.h"
#include <magic_enum.hpp>

#include <fmt/core.h>

namespace game2d {

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
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  const auto& item_tag = r.get<TagComponent>(item_e);

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
  ImGui::SetItemTooltip("%s", item_c.display_name.c_str());
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

/*
template<class T, class ET>
entt::entity
add_item(entt::registry& r,
         entt::entity slot_e,
         std::optional<std::string> display_name = std::nullopt,
         std::optional<ET> type = std::nullopt)
{
  std::optional<T> data_v;
  if (type.has_value())
    data_v = T(type.value());
  else
    data_v = T();
  T data = data.value();

  ItemComponent item_c;
  item_c.display_icon = data.icon;

  if (type.has_value())
    item_c.display_name = std::string(magic_enum::enum_name(type));
  else
    item_c.display_name = display_name.value();

  item_c.parent_slot = slot_e;

  auto item_e = create_empty<ItemComponent>(r, item_c);
  update_item_parent(r, item_e, slot_e);
  return item_e;
};
*/

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

      const auto create_armour = [](entt::registry& r, const entt::entity slot_e, const ArmourType& type) -> entt::entity {
        DataArmour data(type);

        ItemComponent item_c;
        item_c.display_icon = data.icon;
        item_c.display_name = std::string(magic_enum::enum_name(type));
        item_c.parent_slot = slot_e;
        auto item_e = create_empty<ItemComponent>(r, item_c);
        r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::armour });

        update_item_parent(r, item_e, slot_e);
        return item_e;
      };
      const auto create_breachcharge = [](entt::registry& r, const entt::entity slot_e) -> entt::entity {
        DataBreachCharge data;

        ItemComponent item_c;
        item_c.display_icon = data.icon;
        item_c.display_name = "Breach Charge";
        item_c.parent_slot = slot_e;
        auto item_e = create_empty<ItemComponent>(r, item_c);
        r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::bomb });

        update_item_parent(r, item_e, slot_e);
        return item_e;
      };
      const auto create_shotgun = [](entt::registry& r, const entt::entity slot_e) -> entt::entity {
        DataWeaponShotgun data;

        ItemComponent item_c;
        item_c.display_icon = data.icon;
        item_c.display_name = "Shotgun";
        item_c.parent_slot = slot_e;
        auto item_e = create_empty<ItemComponent>(r, item_c);
        r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::gun });

        update_item_parent(r, item_e, slot_e);
        return item_e;
      };
      const auto create_bullets = [](entt::registry& r, const entt::entity slot_e, const BulletType& type) -> entt::entity {
        DataBullet data(type);

        ItemComponent item_c;
        item_c.display_icon = data.icon;
        item_c.display_name = std::string(magic_enum::enum_name(type));
        item_c.parent_slot = slot_e;
        auto item_e = create_empty<ItemComponent>(r, item_c);

        std::optional<ItemType> t = std::nullopt;
        if (type == BulletType::BOUNCY)
          t = ItemType::bullet_bouncy;
        if (type == BulletType::DEFAULT)
          t = ItemType::bullet_default;
        r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ t.value() });

        update_item_parent(r, item_e, slot_e);
        return item_e;
      };

      // init body
      r.get<InventorySlotComponent>(body_c.body[0]).item_e = create_armour(r, body_c.body[0], ArmourType::HEAD);
      r.get<InventorySlotComponent>(body_c.body[1]).item_e = create_armour(r, body_c.body[1], ArmourType::CORE);
      r.get<InventorySlotComponent>(body_c.body[2]).item_e = create_armour(r, body_c.body[2], ArmourType::ARM);
      r.get<InventorySlotComponent>(body_c.body[3]).item_e = create_armour(r, body_c.body[3], ArmourType::ARM);
      r.get<InventorySlotComponent>(body_c.body[4]).item_e = create_armour(r, body_c.body[4], ArmourType::LEG);
      r.get<InventorySlotComponent>(body_c.body[5]).item_e = create_armour(r, body_c.body[5], ArmourType::LEG);
      r.get<InventorySlotComponent>(body_c.body[6]).item_e = create_breachcharge(r, body_c.body[6]);

      // initial items in your inventory
      const auto inv_1_e = inv_c.inv[inv_c.inv.size() - 1];
      const auto inv_2_e = inv_c.inv[inv_c.inv.size() - 2];
      const auto inv_3_e = inv_c.inv[inv_c.inv.size() - 3];
      const auto inv_4_e = inv_c.inv[inv_c.inv.size() - 4];
      const auto inv_5_e = inv_c.inv[inv_c.inv.size() - 5];
      r.get<InventorySlotComponent>(inv_1_e).item_e = create_inv_scrap(r, inv_1_e);
      r.get<InventorySlotComponent>(inv_2_e).item_e = create_shotgun(r, inv_2_e);
      r.get<InventorySlotComponent>(inv_3_e).item_e = create_bullets(r, inv_3_e, BulletType::DEFAULT);
      r.get<InventorySlotComponent>(inv_4_e).item_e = create_bullets(r, inv_4_e, BulletType::BOUNCY);
      r.get<InventorySlotComponent>(inv_5_e).item_e = create_breachcharge(r, inv_5_e);
    }

    // Four cases to handle here.
    // UI: yes. Instance: yes. all good.
    // UI: no. Instance: no. all good.
    // UI: yes. Instance: no. spawn.
    // UI: no. Instance: yes. destroy.
    const auto slots = get_slots(r, e, InventorySlotType::gun);
    const auto ui_gun_slot_e = r.get<InventorySlotComponent>(slots[0]);
    const bool ui_has_gun = ui_gun_slot_e.item_e != entt::null;
    auto* has_weapon = r.try_get<HasWeaponComponent>(e);
    const bool gun_has_instance = has_weapon && has_weapon->instance != entt::null;

    /*
    const bool is_shotgun = ui_has_gun && r.try_get<ShotgunComponent>(ui_gun_slot_e.item_e) != nullptr;
    const bool is_breach_charge = ui_has_gun && r.try_get<BreachChargeComponent>(ui_gun_slot_e.item_e) != nullptr;
    */

    // spawn
    if (ui_has_gun && !gun_has_instance) {
      // create weapon
      // create the correct weapon spawn from inventory item type

      const auto& type = r.get<ItemTypeComponent>(ui_gun_slot_e.item_e);

      if (type.type == ItemType::gun) {
        // fmt::println("adding shotgun components to item");
        add_weapon_shotgun(r, e);
      }

      // breach charge system uses ItemTypeComponent to monitor inventory
      // if (type.type == ItemType::bomb) {
      //   fmt::println("adding breachcharge components to item");
    }

    // destroy
    if (!ui_has_gun && gun_has_instance) {
      auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
      dead.dead.emplace(has_weapon->instance);
      r.remove<HasWeaponComponent>(e);
    }

    // Ammo. Two cases.
    // UI: gun in slot.
    // Instance: must exist.
    // Ammo: must exist in slot.
    // Add/remove abletoshoot component.

    const auto ammo_slots = get_slots(r, e, InventorySlotType::bullet);
    const auto ui_ammo_slot = r.get<InventorySlotComponent>(ammo_slots[0]);
    const bool ui_has_ammo = ui_ammo_slot.item_e != entt::null;

    /*
    const auto* bullet_c = r.try_get<BulletComponent>(ui_ammo_slot.item_e);
    const bool ui_is_ammo = bullet_c != nullptr;
    */

    if (ui_has_gun && gun_has_instance && ui_has_ammo)
      r.emplace_or_replace<AbleToShoot>(has_weapon->instance);
    if (ui_has_gun && gun_has_instance && !ui_has_ammo)
      remove_if_exists<AbleToShoot>(r, has_weapon->instance);

    // Ammo. Set bullet type.
    if (ui_has_gun && gun_has_instance && ui_has_ammo) {

      const auto& type = r.get<ItemTypeComponent>(ui_ammo_slot.item_e);
      // fmt::println("bullet: {}", std::string(magic_enum::enum_name(type.type)));

      std::optional<BulletType> bullet_type = std::nullopt;
      if (type.type == ItemType::bullet_default)
        bullet_type = BulletType::DEFAULT;
      if (type.type == ItemType::bullet_bouncy)
        bullet_type = BulletType::BOUNCY;

      // set the bullet type for shotgun.
      if (auto* shotgun_c = r.try_get<ShotgunComponent>(has_weapon->instance)) {
        if (bullet_type.has_value())
          shotgun_c->bullet_type = bullet_type.value();
      }
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