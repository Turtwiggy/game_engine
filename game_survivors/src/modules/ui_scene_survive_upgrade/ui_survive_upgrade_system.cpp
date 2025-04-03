#include "pch.hpp"

#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "resources/data.hpp"
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
    const auto srgb = hex_to_srgb("#D9D9D9"); //  white
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

struct CardDataUI
{
  Rarity rarity = Rarity::COMMON;
  std::string rarity_txt = "common";
  std::string header_txt = "Bronze Hulls";
  std::string desc_txt = "+15 firerate";
  bool selected = false;
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
#if defined(_DEBUG)
  // auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);
  // if (scene_c.s == Scene::menu) {
  //   gesert_component<SINGLE_XpComponent>(r);
  //   gesert_component<SINGLE_LevelUpUI>(r);
  //   gesert_component<SINGLE_Upgrades>(r);
  //   static bool init = false;
  //   if (!init) {
  //     for (int i = 0; i < 4; i++)
  //       create_empty<PlayerComponent>(r, PlayerComponent{ i });
  //     init = true;
  //   }
  // }
#endif

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
      flags |= ImGuiWindowFlags_NoSavedSettings;

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
      state_c.actions.clear();

      const auto player_e = get_player_e_from_idx(r, i);
      if (player_e == entt::null)
        continue;
      const auto& upgrades_c = r.get<UpgradeResultsComponent>(player_e);

      setup_ui_based_on_upgrades(r, player_e, state_c, upgrades_c);
    }
  }

  // dont show upgrade ui
  ui_c.open = false;
  if (!is_choosing_upgrade(r))
    return;
  ui_c.open = true;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  const auto ui_scale = get_first_component<SINGLE_UIData>(r).scaling;

  const float upg_header_height = 50.0f * ui_scale;
  const int cards = 3;
  const auto card_size = ImVec2{ 225 * ui_scale, 150 * ui_scale };
  const float card_padding_y = 8 * ui_scale; // pad between cards
  const auto header_pad = 4 * ui_scale;
  const auto header_y_size = 30 * ui_scale;
  const auto line_size = 40.0f * ui_scale;
  const float desired_y = (card_size.y * cards) + ((cards - 1) * card_padding_y) + upg_header_height;

  // idx: 3 should be fingerpaint, idx: 4 should be fingerpaint scaled.
  auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[ui_scale == 1.0f ? 3 : 4];
  const auto text_font_enum = ui_scale == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
  auto* font = get_fingerpaint_font(r, text_font_enum);

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const float window_x_size = ri_c.viewport_size_render_at.x;
  const float window_y_size = ri_c.viewport_size_render_at.y;
  const auto set_window_size = ImVec2{ window_x_size, window_y_size };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

  ImGui::Begin("UpgradeUI", nullptr, flags);

  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();
  auto* draw_list = ImGui::GetWindowDrawList();

  // float padding_x = 4;
  // float padding_y = 4;
  const auto player_ui_wh = ImVec2{ ui_wh.x / max_num_players, ui_wh.y };
  auto player_ui_tl = ImVec2{ ui_tl.x, ui_tl.y };
  auto player_ui_br = ImVec2{ ui_tl.x + player_ui_wh.x, ui_tl.y + player_ui_wh.y };

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    const auto player_e = get_player_e_from_idx(r, player_idx);
    if (player_e == entt::null) {

      // move horizontally
      player_ui_tl.x += player_ui_wh.x;
      player_ui_br.x += player_ui_wh.x;

      continue;
    }

    const auto* upgrades_c = r.try_get<UpgradeResultsComponent>(player_e);
    if (!upgrades_c) {

      // move horizontally
      player_ui_tl.x += player_ui_wh.x;
      player_ui_br.x += player_ui_wh.x;

      continue; // this player isnt upgrading
    }

    // update input
    auto& state_c = ui_c.ui_states[player_idx];
    state_c.actions.clear();
    process_input_for_ui(r, state_c, steam_state_c.handles[player_idx]);
    const bool do_act = std::find(state_c.actions.begin(), state_c.actions.end(), UIAction::SELECT) != state_c.actions.end();

    // debug background
    // const auto pad_tl = ImVec2{ player_ui_tl.x, player_ui_tl.y };
    // const auto pad_br = ImVec2{ player_ui_tl.x + player_ui_wh.x, player_ui_tl.y + player_ui_wh.y };
    // draw_list->AddRectFilled(pad_tl, pad_br, IM_COL32((player_idx / 4.0f) * 255, 255, 255, 150), 0);

    const auto clamped_tl =
      ImVec2(player_ui_tl.x + 0.5f * (player_ui_wh.x - card_size.x), player_ui_tl.y + 0.5f * (player_ui_wh.y - desired_y));
    const auto clamped_wh = card_size;

    const auto* head_f = fingerpaint_font;
    const auto* f = font; // body font

    // add upgrade header for column
    const auto im_player_col = convert_my_to_im(default_player_colours[player_idx]);
    const auto upg_text = "Upgrade";
    const auto upg_pos = ImVec2{ clamped_tl.x + 0.5f * clamped_wh.x, clamped_tl.y }; // top center
    const auto upg_center = center_text(head_f, upg_text, upg_pos, { 0.5f, 0.0f });
    draw_list->AddText(head_f, head_f->FontSize, upg_center, im_player_col, upg_text);

    // some separator lines
    const auto s_y = clamped_tl.y + head_f->FontSize * 0.5f;
    const auto l0_p1 = ImVec2{ clamped_tl.x, s_y };
    const auto l0_p2 = ImVec2{ clamped_tl.x + line_size, s_y };
    draw_list->AddLine(l0_p1, l0_p2, im_player_col);
    const auto l1_p1 = ImVec2{ clamped_tl.x + clamped_wh.x - line_size, s_y };
    const auto l1_p2 = ImVec2{ clamped_tl.x + clamped_wh.x, s_y };
    draw_list->AddLine(l1_p1, l1_p2, im_player_col);

    auto card_ui_tl = ImVec2{ clamped_tl.x, clamped_tl.y + upg_header_height };
    auto card_ui_br = ImVec2{ clamped_tl.x + card_size.x, clamped_tl.y + upg_header_height + card_size.y };
    auto card_ui_wh = calc_wh(card_ui_tl, card_ui_br);

    for (int card_idx = 0; card_idx < (int)state_c.rows.size(); card_idx++) {

      // card data.
      const auto [rarity, upgrade] = upgrades_c->results[card_idx];
      const auto rarity_str = std::string(magic_enum::enum_name(rarity));
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));
      const auto [amount, type_str] = stat_from_stat_table(rarity, upgrade);
      const auto desc_txt = std::format("{} {:0.2f}", upgrade_str, amount);

      const CardDataUI data{
        .rarity = rarity,
        .rarity_txt = rarity_str,
        .header_txt = "Bronze Hulls", // todo: generate unique names for all upgradez
        .desc_txt = desc_txt,
        .selected = state_c.current_row_index == card_idx,
      };

      const auto txt_col = IM_COL32(5, 5, 5, 255);
      const auto rounding = 4;
      const auto im_rcol_vec = rarity_to_col(data.rarity);
      const auto im_rcol = IM_COL32(im_rcol_vec.x * 255, im_rcol_vec.y * 255, im_rcol_vec.z * 255, im_rcol_vec.w * 255);

      // card background
      const auto my_card_bg_col = hex_to_srgb("#21242B", 0.75f * 255);
      const auto im_card_bg_col = convert_my_to_im(my_card_bg_col);
      draw_list->AddRectFilled(card_ui_tl, card_ui_br, im_card_bg_col, rounding);
      draw_list->AddRect(card_ui_tl, card_ui_br, im_rcol, rounding, 0, 2.0f);

      // card header background.
      const auto header_bg_tl = ImVec2(card_ui_tl.x + header_pad, card_ui_tl.y + header_pad);
      const auto header_bg_br = ImVec2(card_ui_tl.x + card_ui_wh.x - header_pad, card_ui_tl.y + header_pad + header_y_size);
      const auto header_bg_wh = calc_wh(header_bg_tl, header_bg_br);
      draw_list->AddRectFilled(header_bg_tl, header_bg_br, im_rcol, 0);

      // card header text.
      const auto header_txt_center = center_text(head_f, data.header_txt, calc_center(header_bg_tl, header_bg_wh));
      draw_list->AddText(head_f, head_f->FontSize, header_txt_center, txt_col, data.header_txt.c_str());

      // draw the card rarity.
      const auto rarity_tl = ImVec2(header_bg_tl.x, header_bg_br.y);
      const auto rarity_br = ImVec2(header_bg_br.x, header_bg_br.y + f->FontSize);
      const auto rarity_wh = calc_wh(rarity_tl, rarity_br);
      const auto rarity_txt_center = center_text(f, data.rarity_txt, calc_center(rarity_tl, rarity_wh));
      draw_list->AddText(f, f->FontSize, rarity_txt_center, im_rcol, data.rarity_txt.c_str());

      // draw the card info text.
      const auto desc_tl = ImVec2(card_ui_tl.x, header_bg_br.y);
      const auto desc_br = ImVec2(card_ui_br.x, card_ui_br.y);
      const auto desc_wh = calc_wh(desc_tl, desc_br);
      const auto desc_pos = center_text(f, data.desc_txt, calc_center(desc_tl, desc_wh));
      draw_list->AddText(f, f->FontSize, desc_pos, im_rcol, data.desc_txt.c_str());

      // draw the selected icon bg
      const auto icon_tl = ImVec2(card_ui_br.x - 25, card_ui_br.y - 25);
      const auto icon_br = ImVec2(card_ui_br.x - 5, card_ui_br.y - 5);
      const auto icon_wh = calc_wh(icon_tl, icon_br);
      const auto my_icon_bg_col = hex_to_srgb("#D9D9D9", 0.15f * 255);
      const auto im_icon_bg_col = convert_my_to_im(my_icon_bg_col);
      draw_list->AddRectFilled(icon_tl, icon_br, im_icon_bg_col, rounding);

      // if selected, draw a circle in the box.
      if (data.selected) {
        const auto circle_center = calc_center(icon_tl, icon_wh);
        draw_list->AddCircle(circle_center, 6.0f, im_player_col);
      }

      // Draw a selecable button
      int col_idx = 0;
      SelectableButtonDef def{
        .label = "##aquire_" + rarity_str + "_" + upgrade_str,
        .size = card_ui_wh,
        .input = do_act,
        .my_row_index = card_idx,
        .my_col_index = 0, // one col
        .ui_row_index = ui_c.ui_states[player_idx].current_row_index,
        .ui_col_index = col_idx,
        .ui_col_active = true,

        // hide the buttons (display handled elsewhere)
        .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
        .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
        .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
        .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      };

      ImGui::SetCursorPos(card_ui_tl);
      if (selectable_button(r, def)) {
        // Process action (aquire the upgrade)
        state_c.rows[state_c.current_row_index].action();
        break;
      }

      // move vertically
      card_ui_tl.y += card_size.y + card_padding_y;
      card_ui_br.y += card_size.y + card_padding_y;
    }

    // move horizontally
    player_ui_tl.x += player_ui_wh.x;
    player_ui_br.x += player_ui_wh.x;
  }

  ImGui::End();
  ImGui::PopStyleVar(3);
}

} // namespace game2d