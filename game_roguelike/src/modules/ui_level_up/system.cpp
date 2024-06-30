#pragma once

#include "system.hpp"
#include "modules/player/components.hpp"
#include "modules/rpg_xp/components.hpp"

#include "imgui.h"

#include <fmt/core.h>

namespace game2d {

void
update_ui_level_up_system(const GameEditor& editor, Game& game)
{
  const auto& ri = editor.renderer;
  auto& r = game.state;

  int i = 0;
  for (auto [entity, player, stats, xp] : r.view<PlayerComponent, StatsComponent, XpComponent>().each()) {
    i++;

#ifdef _DEBUG
    ImGui::Begin("Debug");
    std::string label = fmt::format("(debug) Give player {} 50 XP", i);
    if (ImGui::Button(label.c_str()))
      xp.amount += 50;
    ImGui::End();
#endif

    auto* xp_ptr = r.try_get<WantsToLevelUp>(entity);
    if (xp_ptr) {

      // Get existing request, or create a new request
      auto* lv_stat_ptr = r.try_get<WantsToLevelStat>(entity);
      if (!lv_stat_ptr) {
        WantsToLevelStat stat;
        r.emplace<WantsToLevelStat>(entity, stat);
        lv_stat_ptr = r.try_get<WantsToLevelStat>(entity);
      }

      const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
      const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
      const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
      ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

      ImGui::Begin("LevelUp", NULL);

      auto window_table_label = std::format("level-up-table-{}", i).c_str();
      if (ImGui::BeginTable(window_table_label, 3)) {

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        if (ImGui::Button("con\n+4 MAX HP", ImVec2(-FLT_MIN, -FLT_MIN))) {
          lv_stat_ptr->con.push_back(1);
        }

        ImGui::TableNextColumn();
        if (ImGui::Button("str\n+1 ATK", ImVec2(-FLT_MIN, -FLT_MIN))) {
          lv_stat_ptr->str.push_back(1);
        }

        ImGui::TableNextColumn();
        if (ImGui::Button("agi\n+1 DEF", ImVec2(-FLT_MIN, -FLT_MIN))) {
          lv_stat_ptr->agi.push_back(1);
        }

        ImGui::EndTable();
      }
      ImGui::End();
    }
  }
}

} // namespace game2d