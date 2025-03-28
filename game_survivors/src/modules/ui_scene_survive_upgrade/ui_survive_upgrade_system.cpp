#include "pch.hpp"

#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "ui_survive_upgrade_helpers.hpp"

namespace game2d {

// Calculate the sum of values at compile time
constexpr int
sum_array_values()
{
  int sum = 0;
  for (const auto& pair : rarity_chance_map)
    sum += pair.second;
  return sum;
};

const auto stat_from_stat_table = [](Rarity rarity, UpgradeableStat upgrade) -> std::pair<float, std::string> {
  float amount = 0;

  const auto rarity_str = std::string(magic_enum::enum_name(rarity));
  const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));

  if (upgrade == UpgradeableStat::ACTOR_DODGE_CHANCE) {
    if (rarity == Rarity::COMMON)
      amount = 2;
    if (rarity == Rarity::UNCOMMON)
      amount = 4;
    if (rarity == Rarity::RARE)
      amount = 6;
    if (rarity == Rarity::LEGENDARY)
      amount = 8;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 10;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_HEALTH_MAX) {
    if (rarity == Rarity::COMMON)
      amount = 2;
    if (rarity == Rarity::UNCOMMON)
      amount = 5;
    if (rarity == Rarity::RARE)
      amount = 10;
    if (rarity == Rarity::LEGENDARY)
      amount = 15;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 20;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_HEALTH_REGEN) {
    if (rarity == Rarity::COMMON)
      amount = 0.03;
    if (rarity == Rarity::UNCOMMON)
      amount = 0.1;
    if (rarity == Rarity::RARE)
      amount = 0.15;
    if (rarity == Rarity::LEGENDARY)
      amount = 0.25;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 0.4;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_SPEED) {
    if (rarity == Rarity::COMMON)
      amount = 5;
    if (rarity == Rarity::UNCOMMON)
      amount = 10;
    if (rarity == Rarity::RARE)
      amount = 20;
    if (rarity == Rarity::LEGENDARY)
      amount = 35;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }

  /*
  if (upgrade == UpgradeableStat::ACTOR_STAMINA) {
    if (rarity == Rarity::COMMON)
      amount = 1;
    if (rarity == Rarity::UNCOMMON)
      amount = 2;
    if (rarity == Rarity::RARE)
      amount = 3;
    if (rarity == Rarity::LEGENDARY)
      amount = 4;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 5;
    return { amount, "stat_flat_increase" };
  }
  */

  if (upgrade == UpgradeableStat::ACTOR_XP_ZONE_SIZE) {
    if (rarity == Rarity::COMMON)
      amount = 5;
    if (rarity == Rarity::UNCOMMON)
      amount = 15;
    if (rarity == Rarity::RARE)
      amount = 25;
    if (rarity == Rarity::LEGENDARY)
      amount = 35;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 45;
    return { amount, "stat_percent_increase" };
  }

  if (upgrade == UpgradeableStat::BULLET_CRIT_CHANCE) {
    if (rarity == Rarity::COMMON)
      amount = 3;
    if (rarity == Rarity::UNCOMMON)
      amount = 6;
    if (rarity == Rarity::RARE)
      amount = 9;
    if (rarity == Rarity::LEGENDARY)
      amount = 12;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 15;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_CRIT_DAMAGE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 50;
    if (rarity == Rarity::LEGENDARY)
      amount = 75;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 100;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_DAMAGE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_KNOCKBACK) {
    if (rarity == Rarity::COMMON)
      amount = 0.1f;
    if (rarity == Rarity::UNCOMMON)
      amount = 0.2f;
    if (rarity == Rarity::RARE)
      amount = 0.3f;
    if (rarity == Rarity::LEGENDARY)
      amount = 0.4f;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 0.5f;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_LIFESTEAL) {
    if (rarity == Rarity::COMMON)
      amount = 0.1;
    if (rarity == Rarity::UNCOMMON)
      amount = 0.2;
    if (rarity == Rarity::RARE)
      amount = 0.3;
    if (rarity == Rarity::LEGENDARY)
      amount = 0.5;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 1.0;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_PIERCE) {
    if (rarity == Rarity::COMMON)
      amount = 1;
    if (rarity == Rarity::UNCOMMON)
      amount = 2;
    if (rarity == Rarity::RARE)
      amount = 3;
    if (rarity == Rarity::LEGENDARY)
      amount = 4;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 5;
    return { amount, "stat_flat_increase" };
  }

  /*
  if (upgrade == UpgradeableStat::BULLET_SIZE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  */

  if (upgrade == UpgradeableStat::BULLET_SPEED) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }

  if (upgrade == UpgradeableStat::WEAPON_CLIP_SIZE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::WEAPON_FIRERATE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 15;
    if (rarity == Rarity::RARE)
      amount = 25;
    if (rarity == Rarity::LEGENDARY)
      amount = 35;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::WEAPON_RANGE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::WEAPON_RELOAD) {
    if (rarity == Rarity::COMMON)
      amount = -10;
    if (rarity == Rarity::UNCOMMON)
      amount = -20;
    if (rarity == Rarity::RARE)
      amount = -30;
    if (rarity == Rarity::LEGENDARY)
      amount = -40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = -50;
    return { amount, "stat_percent_increase" };
  }

  const auto err_str = std::format("Not impl: {}, {}", rarity_str, upgrade_str);
  throw std::runtime_error(err_str.c_str());
};

const auto rarity_to_col = [](Rarity rarity) -> ImVec4 {
  if (rarity == Rarity::COMMON) {
    const auto srgb = hex_to_srgb("#b1c9c3"); //  gray
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::UNCOMMON) {
    const auto srgb = hex_to_srgb("#00c420"); //  green
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::RARE) {
    const auto srgb = hex_to_srgb("#0096ff"); //  blue
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::LEGENDARY) {
    const auto srgb = hex_to_srgb("#cfc041"); //  gold
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  if (rarity == Rarity::SUPER_LEGENDARY) {
    const auto srgb = hex_to_srgb("#d74200"); //  red
    return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
  }
  return { 1.0f, 1.0f, 1.0f, 1.0f };
};

void
setup_ui_based_on_upgrades(entt::registry& r,
                           entt::entity player_e,
                           UIState& state_c,
                           const UpgradeResultsComponent& upgrades_c)
{
  for (const auto& [rarity, upgrade] : upgrades_c.results) {

    const auto aquire_action = [&r, player_e, rarity, upgrade]() {
      const auto rarity_str = std::string(magic_enum::enum_name(rarity));
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));
      const auto [amount, type_str] = stat_from_stat_table(rarity, upgrade);

      auto& stats_c = r.get<StatModifierComponent>(player_e);
      if (type_str == "stat_flat_increase")
        stats_c.add(std::make_shared<StatFlatIncrease>(amount, upgrade_str));
      else if (type_str == "stat_percent_increase")
        stats_c.add(std::make_shared<StatPercentIncrease>(amount, upgrade_str));
      else
        throw std::runtime_error("Unknown stat type");

      SDL_Log("Aquiring: %s %s", rarity_str.c_str(), upgrade_str.c_str());
      r.remove<UpgradeResultsComponent>(player_e); // done
    };

    state_c.rows.push_back(RowState{ .col_name = "Aquire", .action = aquire_action });
  }
};

void
update_ui_survive_upgrade_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);
  GET_FIRST_OR_RETURN(SINGLE_LevelUpUI, r, ui_e, ui_c);
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, up_e, up_c);
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)

  // check the probabilities are mathing to 100%
  static_assert(sum_array_values() == 100);

#if defined(_DEBUG)
  // Cheats..!! CHEATSS!!! CHEEEATTTSSSSSSS!!!!!!!
  {
    auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
    auto cheat_levelup_state = gesert_menubar_state(menu_c, "Cheat LevelUp");
    if (cheat_levelup_state.enabled) {

      ImGui::SetNextWindowPos(ImVec2{ (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y },
                              ImGuiCond_Always,
                              { 1.0f, 1.0f });
      ImGui::SetNextWindowSize({ 200, 100 });

      ImGuiWindowFlags flags = 0;
      flags |= ImGuiWindowFlags_NoDecoration;
      flags |= ImGuiWindowFlags_NoDocking;
      flags |= ImGuiWindowFlags_NoMove;

      ImGui::Begin("CheatLevelUp", nullptr, flags);

      if (ImGui::Button("LevelUp"))
        sxp_c.xp += sxp_c.xp_for_next_level;

      ImGui::End();
    }
  }
#endif

  const int max_num_players = 4;
  const int num_active_players = non_zero_handles(steam_state_c.handles).size();
  const auto text_col = ImVec4(0.64f, 0.64f, 0.64f, 1.0f);

  if (ui_c.ui_states.size() == 0)
    ui_c.ui_states.resize(max_num_players);

  if (sxp_c.xp >= sxp_c.xp_for_next_level) {
    // consume xp
    sxp_c.xp = 0;
    sxp_c.level++;
    sxp_c.xp_for_next_level += 2; // 2 harder every time

    generate_upgrades_for_players(r, ui_c);

    // reset ui
    for (int i = 0; i < max_num_players; i++) {
      auto& state_c = ui_c.ui_states[i];
      state_c.current_row_index = 0;
      state_c.rows.clear();
      state_c.new_actions.clear();

      const auto player_e = get_player_e_from_idx(r, i);
      if (player_e == entt::null)
        continue;
      auto& upgrades_c = r.get<UpgradeResultsComponent>(player_e);

      setup_ui_based_on_upgrades(r, player_e, state_c, upgrades_c);
    }
  }

  // dont show upgrade ui
  if (!is_choosing_upgrade(r))
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const auto set_window_size = ImVec2{ (float)ri_c.viewport_size_render_at.x - 0.1f * ri_c.viewport_size_render_at.x, 200 };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);

  // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

  ImGui::Begin("UpgradeUI", nullptr, flags);

  const ImVec2 window_pos = ImGui::GetWindowPos();
  const ImVec2 window_size = ImGui::GetWindowSize();
  const auto player_ui_w = window_size.x / max_num_players; // always /4
  const auto player_ui_h = window_size.y;

  float padding_x = 4;
  float padding_y = 4;
  auto player_ui_tl = ImVec2{ window_pos.x, window_pos.y };
  auto player_ui_br = ImVec2{ window_pos.x + player_ui_w, window_pos.y + player_ui_h };

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    const auto player_e = get_player_e_from_idx(r, player_idx);
    if (player_e == entt::null) {

      // move horizontally
      player_ui_tl.x += player_ui_w;
      player_ui_br.x += player_ui_w;

      continue;
    }

    const auto* upgrades_c = r.try_get<const UpgradeResultsComponent>(player_e);
    if (!upgrades_c) {

      // move horizontally
      player_ui_tl.x += player_ui_w;
      player_ui_br.x += player_ui_w;

      continue; // this player isnt upgrading
    }

    // update input
    auto& state_c = ui_c.ui_states[player_idx];
    state_c.new_actions.clear();
    process_input_for_ui(r, state_c, steam_state_c.handles[player_idx]);

    auto pad_tl = ImVec2{ player_ui_tl.x + padding_x, player_ui_tl.y + padding_y };
    auto pad_br = ImVec2{ player_ui_tl.x + player_ui_w - padding_x, player_ui_tl.y + player_ui_h - padding_y };

    // background
    const float inc = ((player_idx + 1) / 4.0f);
    const auto im_active_col = IM_COL32(0, 0, 255 * inc, 255);
    ImGui::GetWindowDrawList()->AddRectFilled(pad_tl, pad_br, im_active_col, 0);

    const bool do_act =
      std::find(state_c.new_actions.begin(), state_c.new_actions.end(), UIAction::SELECT) != state_c.new_actions.end();

    auto y = pad_tl.y;

    // draw upgrades
    for (int i = 0; i < (int)state_c.rows.size(); i++) {
      const bool selected = state_c.current_row_index == i;
      int& selected_idx = state_c.current_row_index;

      // Upgrade info
      const auto [rarity, upgrade] = upgrades_c->results[i];
      const auto rarity_str = std::string(magic_enum::enum_name(rarity));
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));
      const auto [amount, type_str] = stat_from_stat_table(rarity, upgrade);

      auto col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
      if (selected)
        col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

      int col_idx = 0;

      auto def = SelectableButtonDef{
        .label = "Aquire##" + rarity_str + "_" + upgrade_str,
        .size = { 60, 24 },
        .input = do_act,
        .my_row_index = i,
        .my_col_index = 0, // one col
        .ui_row_index = ui_c.ui_states[player_idx].current_row_index,
        .ui_col_index = col_idx, // one col
        .ui_col_active = true,   // one col
      };

      ImGui::SetCursorScreenPos({ pad_tl.x, y });
      if (selectable_button(r, def) || (selected && do_act)) {
        state_c.rows[i].action(); // get it
        break;
      }

      // Display rarity.
      auto rarity_col = rarity_to_col(rarity);
      ImGui::SetCursorScreenPos({ pad_tl.x + def.size.x, y });
      ImGui::TextColored(rarity_col, "%s", rarity_str.c_str());

      // Display upgrade info
      ImGui::SetCursorScreenPos({ pad_tl.x + def.size.x, y + 12 });

      if (type_str == "stat_flat_increase") {
        auto str = std::format("{} +{:.2f}", upgrade_str, amount);
        ImGui::Text("%s", str.c_str());
      } else {
        auto str = std::format("{} {:.2f}%", upgrade_str, amount);
        ImGui::Text("%s", str.c_str());
      }

      // move vertically
      y += def.size.y;
    }

    // move horizontally
    player_ui_tl.x += player_ui_w;
    player_ui_br.x += player_ui_w;
  }

  ImGui::End();
  ImGui::PopStyleVar();
}

} // namespace game2d