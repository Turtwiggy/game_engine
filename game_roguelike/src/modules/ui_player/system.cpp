#include "system.hpp"

#include "components/actors.hpp"
#include "components/events.hpp"
#include "helpers.hpp"
#include "helpers/check_equipment.hpp"
#include "modules/combat/components.hpp"
#include "modules/items/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/player/components.hpp"
#include "modules/rpg_xp/components.hpp"
#include "modules/ux_hover/components.hpp"
#include "renderer/components.hpp"

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

        // assume all the compacted items have the same state
        // this could break if potions had internal state,
        // e.g. usages left
        const auto& entity_item = entity_items[0];

        if (auto* equipment = r.try_get<Equipment>(entity_item)) {
          const auto ent_id = static_cast<uint32_t>(entity_item);
          const auto ent_id_as_str = std::to_string(ent_id);
          const auto id_lhand = std::string("l-hand-") + ent_id_as_str;
          const auto id_rhand = std::string("r-hand-") + ent_id_as_str;

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
            if (ImGui::ImageButton(id_lhand.c_str(), tex_id, icon_size, l_hand_uv[0], l_hand_uv[1])) {
              auto& u = r.get_or_emplace<WantsToEquip>(entity);
              u.requests.push_back({ EquipmentSlot::left_hand, entity_item });
            }
          }
          ImGui::SameLine();
          {
            if (ImGui::ImageButton(id_rhand.c_str(), tex_id, icon_size, r_hand_uv[0], r_hand_uv[1])) {
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