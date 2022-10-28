#include "system.hpp"

#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"
#include "helpers.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_player_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  ImGui::Begin("Player UI");

  // Player Info
  for (auto [entity, player, stats, xp] : r.view<PlayerComponent, StatsComponent, XpComponent>().each()) {
    ImGui::Text("¬¬ Player X ¬¬");
    draw_healthbar(game, entity);
    ImGui::Text("LEVEL: %i", stats.overall_level);
    ImGui::Text("CON: %i", stats.con_level);
    ImGui::Text("STR: %i", stats.str_level);
    ImGui::Text("AGI: %i", stats.agi_level);
    ImGui::Text("You have %i xp", xp.amount);
    ImGui::Separator();

    if (ImGui::Button("(debug) Give 50 XP"))
      xp.amount += 50;

    auto* xp_ptr = r.try_get<WantsToLevelUp>(entity);
    if (xp_ptr) {
      ImGui::Text("Level Up Required!");

      auto* lv_stat_ptr = r.try_get<WantsToLevelStat>(entity);
      if (lv_stat_ptr) {
        if (ImGui::Button("(CON)"))
          lv_stat_ptr->con.push_back(1);
        if (ImGui::Button("(STR)"))
          lv_stat_ptr->str.push_back(1);
        if (ImGui::Button("(AGI)"))
          lv_stat_ptr->agi.push_back(1);
      } else {
        WantsToLevelStat stat;
        if (ImGui::Button("(CON)"))
          stat.con.push_back(1);
        if (ImGui::Button("(STR)"))
          stat.str.push_back(1);
        if (ImGui::Button("(AGI)"))
          stat.agi.push_back(1);
        r.emplace<WantsToLevelStat>(entity, stat);
      }
    }
  }

  ImGui::End();
}

} // namespace game2d