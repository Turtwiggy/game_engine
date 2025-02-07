#include "modules/ui_scene_survive_level_up/ui_survive_level_up_system.hpp"

#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "ui_survive_level_up_components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/events_components.hpp"
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
  GET_FIRST_OR_RETURN(SINGLE_LevelUpUI, r, ui_e, ui_c);
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, up_e, up_c);
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  const bool level_up_required = sxp_c.xp >= sxp_c.xp_for_next_level;

  // Cheats..!! CHEATSS!!! CHEEEATTTSSSSSSS!!!!!!!
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

  ui_c.require_level_up = level_up_required;
  if (!level_up_required)
    return;

  process_input_for_ui(r, ui_c.state);
  bool& do_act = ui_c.state.do_action;
  int& selected = ui_c.state.selected;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({ 640, 360 }, ImGuiCond_Always);

  auto available_upgrades = available_upgrade_names(r);
  int index = 0;
  const float size_x = ImGui::CalcTextSize("Aquire").x;
  const ImVec2 size = { size_x, 13.0f };

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

  const auto& view = r.view<PlayerComponent, StatModifierComponent, TraitComponent>();
  for (const auto& [e, player_c, stat_c, trait_c] : view.each()) {

    auto& upgrade_c = r.get_or_emplace<UpgradeComponent>(e);
    auto& aquired_upgrades = upgrade_c.aquired_upgrades;
    const auto diff = difference(available_upgrades, aquired_upgrades);

    {
      auto def = SelectableButtonDef{
        .label = "AquireAll",
        .size = size,
        .index = index++,
        .input = do_act,
        .sel_index = selected,
      };
      if (selectable_button(def)) {
        for (const std::string& u : diff) {
          auto upgrade = find_upgrade(r, u);
          UpgradeEvent evt;
          evt.e = e;
          evt.upgrade = upgrade;
          evts_c.dispatcher->trigger(evt);
          evts_c.dispatcher->update();
        }

        close_ui();
      }
    }

    for (const std::string& u : diff) {
      auto upgrade = find_upgrade(r, u);

      auto def = SelectableButtonDef{
        .label = "Aquire##" + u,
        .size = size,
        .index = index++,
        .input = do_act,
        .sel_index = selected,
      };

      if (selectable_button(def)) {
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
  {
    auto full_hp_def = SelectableButtonDef{
      .label = "Aquire##RefilHp",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    if (selectable_button(full_hp_def)) {
      const auto& players_view = r.view<PlayerFixtureComponent, HealthComponent>();
      for (const auto& [e, player_c, hp_c] : players_view.each())
        hp_c.hp = hp_c.max_hp;

      close_ui();
    }
    ImGui::SameLine();
    ImGui::TextColored(text_col, "%s", "Full HP");
  }

  // TEMP: increase hp option
  {
    auto increase_hp_def = SelectableButtonDef{
      .label = "Aquire##IncreaseHp",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    if (selectable_button(increase_hp_def)) {
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
  }

  // TEMP: increase damage option
  {
    auto def = SelectableButtonDef{
      .label = "Aquire##IncreaseDamage",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    if (selectable_button(def)) {
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
  }

  // TEMP: collect all xp
  {
    auto def = SelectableButtonDef{
      .label = "Aquire##CollectAllXp (BROKEN)",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    if (selectable_button(def)) {

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
    ImGui::TextColored(text_col, "%s", "Better FPS. Collect all XP on the map.");
  }

  // TEMP: close without upgrading
  {
    auto def = SelectableButtonDef{
      .label = "LeveLUp",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    if (selectable_button(def))
      close_ui();
    ImGui::SameLine();
    ImGui::TextColored(text_col, "%s", "Close - without upgrading");
  }

  ui_c.state.max = index;
  ImGui::End();
}

} // namespace game2d