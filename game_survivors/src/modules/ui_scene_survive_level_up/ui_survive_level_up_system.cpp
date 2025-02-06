#include "modules/ui_scene_survive_level_up/ui_survive_level_up_system.hpp"

#include "ui_survive_level_up_components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_events/events_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/event_upgrade/event_upgrade_components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade/upgrade_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

#include <imgui.h>
#include <magic_enum.hpp>

namespace game2d {

std::vector<std::string>
difference(std::vector<std::string>& a, std::vector<std::string>& b)
{
  // Sort the vectors (required for std::set_difference)
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());

  std::vector<std::string> diff;
  std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(diff));
  return diff;
};

void
update_ui_survive_level_up_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri);
  GET_FIRST_OR_RETURN(SINGLE_LevelUpUI, r, uiup_e, uiup_c);
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, up_e, up_c);
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  bool level_up_required = sxp_c.xp >= sxp_c.xp_for_next_level;

  {
    auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
    auto cheat_levelup_state = gesert_menubar_state(menu_c, "Cheat LevelUp");
    if (cheat_levelup_state.enabled) {
      ImGui::Begin("CheatLevelUp");
      if (ImGui::Button("LevelUp"))
        sxp_c.xp += sxp_c.xp_for_next_level;
      ImGui::End();
    }
  }

  uiup_c.require_level_up = level_up_required;
  if (!level_up_required)
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({ 640, 360 }, ImGuiCond_Always);

  auto close_ui = [&sxp_c]() {
    sxp_c.xp = 0;
    sxp_c.level++;
    sxp_c.xp_for_next_level += 5; // 5 harder every time
  };

  //
  // given the total list of upgrades, e.g. 100 upgrades,
  // and the upgrades that your character has unlocked, e.g. 3 upgrades,
  // get X random upgrades that your character does not have.
  //

  ImGui::Begin("Level up required!", NULL, flags);
  ImGui::Text("Level-up!");
  const auto text_col = ImVec4(0.64f, 0.64f, 0.64f, 1.0f);

  auto available_upgrades = available_upgrade_names(r);

  const auto& view = r.view<PlayerComponent, StatModifierComponent, TraitComponent>();
  for (const auto& [e, player_c, stat_c, trait_c] : view.each()) {
    auto& upgrade_c = r.get_or_emplace<UpgradeComponent>(e);
    auto& aquired_upgrades = upgrade_c.aquired_upgrades;
    const auto diff = difference(available_upgrades, aquired_upgrades);

    for (const std::string& u : diff) {
      auto upgrade = find_upgrade(r, u);

      std::string label = "Aquire##" + u;
      if (ImGui::Button(label.c_str())) {
        UpgradeEvent evt;
        evt.e = e;
        evt.upgrade = upgrade;
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();

        close_ui();
      }

      // Name
      ImGui::SameLine();
      ImGui::Text("%s", u.c_str());

      // Description
      ImGui::SameLine();
      const std::string desc = generate_description(upgrade);
      ImGui::TextColored(text_col, "%s", desc.c_str());
    }
  }

  // TEMP: full hp option
  if (ImGui::Button("Aquire##RefillHp")) {
    const auto& players_view = r.view<PlayerFixtureComponent, HealthComponent>();
    for (const auto& [e, player_c, hp_c] : players_view.each())
      hp_c.hp = hp_c.max_hp;
    close_ui();
  }
  ImGui::SameLine();
  ImGui::TextColored(text_col, "%s", "Full HP");

  // TEMP: increase hp option
  if (ImGui::Button("Aquire##IncreaseHp")) {
    const auto stat = UpgradeableStat::ACTOR_MAX_HEALTH;
    const auto stat_key = std::string(magic_enum::enum_name(stat));
    int max_hp_amount = 5;
    const auto& players_view = r.view<StatModifierComponent>();
    for (const auto& [e, stat_c] : players_view.each())
      stat_c.add(std::make_shared<StatFlatIncrease>(max_hp_amount, stat_key));
    close_ui();
  }
  ImGui::SameLine();
  ImGui::TextColored(text_col, "%s", "+5 Max HP");

  // TEMP: increase damage option
  if (ImGui::Button("Aquire##IncreaseDamage")) {
    const auto stat = UpgradeableStat::BULLET_DAMAGE;
    const auto stat_key = std::string(magic_enum::enum_name(stat));
    const int amount = 10;
    const auto& players_view = r.view<StatModifierComponent>();
    for (const auto& [e, stat_c] : players_view.each())
      stat_c.add(std::make_shared<StatFlatIncrease>(amount, stat_key));
    close_ui();
  }
  ImGui::SameLine();
  ImGui::TextColored(text_col, "%s", "+10 Bullet Damage");

  // TEMP: collect all xp
  if (ImGui::Button("Aquire##CollectAllXp (BROKEN)")) {

    const auto& xp_view = r.view<XpComponent>();
    auto& xp_c = get_first_component<SINGLE_XpComponent>(r);
    int xp_amount = xp_view.size();
    xp_c.xp += xp_amount;

    // Kill all the xp...
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

    for (const auto& [e, xp_c] : xp_view.each())
      dead.dead.emplace(e);

    close_ui();
  }
  ImGui::SameLine();
  ImGui::TextColored(text_col, "%s", "Collect all XP on the map.");

  /*
  if (ImGui::BeginTable(label.c_str(), 3)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    if (ImGui::Button("Not Impl", ImVec2(-FLT_MIN, -FLT_MIN))) {
      close_ui();
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Not Impl", ImVec2(-FLT_MIN, -FLT_MIN))) {
      close_ui();
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Not Impl", ImVec2(-FLT_MIN, -FLT_MIN))) {
      close_ui();
    }

    ImGui::EndTable();
  }
  */

  std::string label = std::format("LeveUp");
  if (ImGui::Button("Close without upgrading")) {
    close_ui();
  }

  ImGui::End();
}

} // namespace game2d