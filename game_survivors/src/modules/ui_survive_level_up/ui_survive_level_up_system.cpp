#include "modules/ui_survive_level_up/ui_survive_level_up_system.hpp"

#include "engine/entt/helpers.hpp"
#include "imgui.h"
#include "magic_enum.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "ui_survive_level_up_components.hpp"

namespace game2d {

void
update_ui_survive_level_up_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri);
  GET_FIRST_OR_RETURN(SINGLE_LevelUpUI, r, lvup_e, lvup_c);

  bool level_up_required = sxp_c.xp >= sxp_c.xp_for_next_level;

#if defined(_DEBUG)
  if (ImGui::Button("GiveXp"))
    sxp_c.xp += 34;
#endif

  lvup_c.require_level_up = level_up_required;
  if (!level_up_required)
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({ 640, 360 }, ImGuiCond_Always);

  ImGui::Begin("Level up required!", NULL, flags);
  ImGui::Text("Level-up!");

  std::string label = std::format("LeveUp");

  if (ImGui::BeginTable(label.c_str(), 3)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    if (ImGui::Button("+HP", ImVec2(-FLT_MIN, -FLT_MIN))) {
      sxp_c.xp = 0;
      sxp_c.level++;
      sxp_c.xp_for_next_level += 5; // 5 harder every time

      // TEMP: refill hp.
      const auto& players_view = r.view<PlayerFixtureComponent>();
      for (const auto& [e, player_c] : players_view.each()) {
        if (auto* hp_c = r.try_get<HealthComponent>(e))
          hp_c->hp = hp_c->max_hp;
      }
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("+DMG", ImVec2(-FLT_MIN, -FLT_MIN))) {
      sxp_c.xp = 0;
      sxp_c.level++;
      sxp_c.xp_for_next_level += 5; // 5 harder every time

      const auto& players_view = r.view<PlayerComponent, HasWeaponsComponent>();
      for (const auto& [e, player_c, has_weps_c] : players_view.each()) {
        for (const auto wep_e : has_weps_c.weapons) {
          auto& bullet_damage_c = r.get<BulletDamage>(wep_e);
          bullet_damage_c.damage += 5;
        }
      }

      //
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("+1 Bullet Pierce", ImVec2(-FLT_MIN, -FLT_MIN))) {
      sxp_c.xp = 0;
      sxp_c.level++;
      sxp_c.xp_for_next_level += 5; // 5 harder every time

      const auto stat = UpgradeableStat::BULLET_PIERCE;
      const auto stat_key = std::string(magic_enum::enum_name(stat));

      const auto& players_view = r.view<StatModifierComponent>();
      for (const auto& [e, stat_c] : players_view.each()) {
        int mod_val = 1;
        stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, stat_key));
      }

      //
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

} // namespace game2d