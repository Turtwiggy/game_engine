#include "system.hpp"

#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/helpers/check_equipment.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/items/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"
#include "helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ux_hover/components.hpp"
#include "resources/textures.hpp"

#include "magic_enum.hpp"
#include <entt/entt.hpp>
#include <imgui.h>

#include <map>
#include <string>

namespace game2d {

void
update_ui_player_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoMove;

  ImGuiWindowClass window_class;
  window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
  ImGui::SetNextWindowClass(&window_class);

  static bool show_player = true;
  ImGui::Begin("Player", &show_player, flags);
  ImGui::Text("FPS: %0.2f", ImGui::GetIO().Framerate);

  {
    // Player Info
    for (auto [entity, player, stats, xp] : r.view<PlayerComponent, StatsComponent, XpComponent>().each()) {
      ImGui::Text("¬¬ Player ¬¬");
      draw_healthbar(game, entity);
      ImGui::Text("LEVEL: %i", stats.overall_level);
      ImGui::Text("CON: %i", stats.con_level);
      ImGui::Text("STR: %i", stats.str_level);
      ImGui::Text("AGI: %i", stats.agi_level);
      ImGui::Text("You have %i xp", xp.amount);

      // if (ImGui::Button("(debug) Give 50 XP"))
      //   xp.amount += 50;

      auto* xp_ptr = r.try_get<WantsToLevelUp>(entity);
      if (xp_ptr) {
        ImGui::Text("Level Up Required!");

        auto* lv_stat_ptr = r.try_get<WantsToLevelStat>(entity);
        if (lv_stat_ptr) {
          if (ImGui::Button("(CON)"))
            lv_stat_ptr->con.push_back(1);
          ImGui::SameLine();
          if (ImGui::Button("(STR)"))
            lv_stat_ptr->str.push_back(1);
          ImGui::SameLine();
          if (ImGui::Button("(AGI)"))
            lv_stat_ptr->agi.push_back(1);
        } else {
          WantsToLevelStat stat;
          if (ImGui::Button("(CON)"))
            stat.con.push_back(1);
          ImGui::SameLine();
          if (ImGui::Button("(STR)"))
            stat.str.push_back(1);
          ImGui::SameLine();
          if (ImGui::Button("(AGI)"))
            stat.agi.push_back(1);
          r.emplace<WantsToLevelStat>(entity, stat);
        }
      }

      // UI: selecting units for an item
      // if (auto* selecting = r.try_get<WantsToSelectUnitsForItem>(entity)) {
      //   ImGui::Text("¬¬ Select Targets for Item ¬¬");
      //   for (auto& item : selecting->items) {
      //     ImGui::Text("Item: %s", r.get<TagComponent>(item.entity).tag.c_str());
      //     for (const auto& target : item.targets)
      //       ImGui::Text("Target: %s", r.get<TagComponent>(target).tag.c_str());
      //     if (ImGui::Button("Clear Targets")) {
      //       item.targets.clear();                    // item state
      //       r.clear<CollidingWithCursorComponent>(); // coll state
      //     }
      //     ImGui::Separator();
      //     std::string label_confirm = "Confirm Use##" + std::to_string(static_cast<uint32_t>(item.entity));
      //     std::string label_cancel = "Cancel Use##" + std::to_string(static_cast<uint32_t>(item.entity));
      //     // If confirm: apply item to targets
      //     if (ImGui::Button(label_confirm.c_str())) {
      //       {
      //         auto& u = r.get_or_emplace<WantsToUse>(entity);
      //         Use info;
      //         info.entity = item.entity;
      //         for (const auto& target : item.targets)
      //           info.targets.push_back(target);
      //         u.items.push_back(info);
      //       }
      //       r.remove<WantsToSelectUnitsForItem>(entity);
      //     }
      //     // If cancel: dont select any units
      //     else if (ImGui::Button(label_cancel.c_str())) {
      //       r.remove<WantsToSelectUnitsForItem>(entity);
      //     }
      //   }
      // }

      ImGui::Separator();
      ImGui::Text("Equipped Items:");
      for (int i = 0; i < magic_enum::enum_count<EquipmentSlot>(); i++) {
        EquipmentSlot slot = magic_enum::enum_value<EquipmentSlot>(i);
        entt::entity equipped = has_equipped(game, entity, slot);

        std::string type_str = "[empty]";
        if (equipped != entt::null) {
          // const auto& info = r.get<const IsEquipped>(equipped);
          const auto& type = r.get<const EntityTypeComponent>(equipped);
          type_str = std::string(magic_enum::enum_name(type.type));
        }

        // cut this sprite in half
        const auto& slots = editor.textures;
        const ImTextureID tex_id = (ImTextureID)get_tex_id(slots, AvailableTexture::kenny);
        const ImVec2 icon_size = { 8.0f, 16.0f };
        const int spr_x = 42;
        const int spr_y = 0;
        auto l_hand_uv = convert_sprite_to_uv(editor, game, spr_x, spr_y);
        l_hand_uv[1].x = { (static_cast<float>(spr_x) * 16 + 8) / 768 };
        auto r_hand_uv = convert_sprite_to_uv(editor, game, spr_x, spr_y);
        r_hand_uv[0].x = { (static_cast<float>(spr_x) * 16 + 8) / 768 };
        if (slot == EquipmentSlot::left_hand)
          ImGui::Image(tex_id, icon_size, l_hand_uv[0], l_hand_uv[1]);
        if (slot == EquipmentSlot::right_hand)
          ImGui::Image(tex_id, icon_size, r_hand_uv[0], r_hand_uv[1]);
        ImGui::SameLine();
        ImGui::Text("%s", type_str.c_str());

        // able to unequip item?
        // if (equipped != entt::null) {
        //   ImGui::SameLine();
        //   const std::string eid = std::to_string(static_cast<uint32_t>(equipped));
        //   const std::string label = "Unequip##" + eid;
        //   if (ImGui::Button(label.c_str())) {
        //     // TODO: want to unequip?
        //   }
        // }
      }

      const auto& equipped = r.view<const IsEquipped, const TagComponent, const EntityTypeComponent>();
      for (auto [entity, equipped, tag, type] : equipped.each()) {
        if (equipped.parent != entity)
          continue; // not my equipped
      }

      ImGui::Text("Inventory");

      const auto& items = r.view<const InBackpackComponent, const TagComponent, const EntityTypeComponent>();

      // Show like potion x1, potion x2 not potions individually
      std::map<std::string, std::vector<entt::entity>> compacted_items;
      for (auto [entity_item, backpack, tag, type] : items.each()) {
        if (backpack.parent != entity)
          continue; // not my item
        compacted_items[tag.tag].push_back(entity_item);
      }

      for (const auto& [tag, entity_items] : compacted_items) {

        const auto& entity_item = entity_items[0];

        if (auto* equipment = r.try_get<Equipment>(entity_item)) {

          // cut this sprite in half
          const auto& slots = editor.textures;
          const ImTextureID tex_id = (ImTextureID)get_tex_id(slots, AvailableTexture::kenny);
          const ImVec2 icon_size = { 8, 16 };
          const int spr_x = 42;
          const int spr_y = 0;
          auto l_hand_uv = convert_sprite_to_uv(editor, game, spr_x, spr_y);
          l_hand_uv[1].x = { (static_cast<float>(spr_x) * 16 + 8) / 768 };
          auto r_hand_uv = convert_sprite_to_uv(editor, game, spr_x, spr_y);
          r_hand_uv[0].x = { (static_cast<float>(spr_x) * 16 + 8) / 768 };

          ImGui::Text("Equip ");
          ImGui::SameLine();
          {
            char l_hand_buffer[64];
            sprintf(l_hand_buffer, "lhand_x%iy%i", spr_x, spr_y);
            if (ImGui::ImageButton(l_hand_buffer, tex_id, icon_size, l_hand_uv[0], l_hand_uv[1])) {
              auto& u = r.get_or_emplace<WantsToEquip>(entity);
              u.requests.push_back({ EquipmentSlot::left_hand, entity_item });
            }
          }
          ImGui::SameLine();
          {
            char r_hand_buffer[64];
            sprintf(r_hand_buffer, "rhand_x%iy%i", spr_x, spr_y);
            if (ImGui::ImageButton(r_hand_buffer, tex_id, icon_size, r_hand_uv[0], r_hand_uv[1])) {
              auto& u = r.get_or_emplace<WantsToEquip>(entity);
              u.requests.push_back({ EquipmentSlot::right_hand, entity_item });
            }
          }
        }
        if (auto* usable = r.try_get<ConsumableComponent>(entity_item)) {
          std::string label_use = "use##" + std::to_string(static_cast<uint32_t>(entity_item));
          if (ImGui::Button(label_use.c_str()))
            use_item(editor, game, entity, entity_item);
        }

        // ImGui::SameLine();
        // std::string label_drop = "Drop##" + std::to_string(static_cast<uint32_t>(entity_item));
        // if (ImGui::Button(label_drop.c_str())) {
        //   auto& u = r.get_or_emplace<WantsToDrop>(entity);
        //   u.items.push_back(entity_item);
        // }
        // ImGui::SameLine();
        // std::string label_sell = "Sell##" + std::to_string(static_cast<uint32_t>(entity_item));
        // if (ImGui::Button(label_sell.c_str())) {
        //   auto& u = r.get_or_emplace<WantsToEquip>(entity);
        //   u.items.push_back(entity_item);
        // }
        ImGui::SameLine();
        ImGui::Text("%s (x%i)", tag.c_str(), entity_items.size()); // label
      }
    }
  }
  ImGui::End();
}

} // namespace game2d