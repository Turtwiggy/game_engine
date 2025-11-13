#include "pch.hpp"

#include "ui_survive_upgrade_components.hpp"
#include "ui_survive_upgrade_helpers.hpp"
#include "ui_survive_upgrade_system.hpp"

#if !defined(_DEBUG)
#include "modules/steam_input/steam_input_helpers.hpp"
#endif
#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/sprites/helpers.hpp"
#include "engine/std/string/helpers.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_helpers.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_select/scene_select_components.hpp"
#include "modules/ui/ui_scene_select/scene_select_helpers.hpp"
#include "modules/ui/ui_scene_survive_onboarding/ui_survive_onboarding_helpers.hpp"
#include "resources/data.hpp"

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

std::vector<DisplayStat>
get_display_stats(entt::registry& r)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);

  std::vector<DisplayStat> display_stats;

  for (int i = 0; i < magic_enum::enum_count<UpgradeableStat>(); i++) {
    if (i == (int)UpgradeableStat::count)
      continue;
    auto key = std::string(magic_enum::enum_name(UpgradeableStat(i)));
    auto clean_key = key;
    clean_key = str_remove_all_occurances(clean_key, "ACTOR_");
    clean_key = str_remove_all_occurances(clean_key, "BULLET_");
    clean_key = str_remove_all_occurances(clean_key, "WEAPON_");
    clean_key = str_remove_all_occurances(clean_key, "AREA_");

    display_stats.push_back({ .key = clean_key, .val = "0" });
  }

  return display_stats;
};

void
draw_upgrade_selections_in_grid(entt::registry& r,
                                ImVec2 selection_tl,
                                ImVec2 selection_br,
                                UIState& state_c,
                                SINGLE_LevelUpUI& ui_c,
                                const UpgradeResultsComponent* upgrades_c,
                                const int player_idx,
                                const bool do_act,
                                const float dt,
                                ImFont* font,
                                const float font_size)
{
  const auto& upg_name_c = get_first_component<SINGLE_UpgradeToName>(r);
  auto* draw_list = ImGui::GetWindowDrawList();
  ImVec2 selection_wh = selection_br - selection_tl;
  const auto upgrades_vec = std::vector<UpgradeRollResult>{ upgrades_c->results.begin(), upgrades_c->results.end() };
  float icon_size = 32.0f;
  float padding_x = 4.0f;
  float first_x = selection_tl.x + 0.5f * (selection_wh.x - ((int)upgrades_vec.size() * icon_size));
  float center_y = selection_tl.y + 0.5f * (selection_wh.y - icon_size);
  float top_y = center_y - 0.5f * icon_size;

  for (int i = 0; i < upgrades_vec.size(); i++) {

    const auto cell_it = std::find(state_c.cells.begin(), state_c.cells.end(), state_c.active);
    const auto cell_idx = static_cast<int>(cell_it - state_c.cells.begin());
    const auto selected = cell_idx == i;

    const UpgradeRollResult result = upgrades_vec[i];
    const auto rarity = result.rarity;
    const auto rarity_str = std::string(magic_enum::enum_name(rarity));
    const auto im_rcol_vec = rarity_to_col(rarity);
    const auto im_rcol = IM_COL32(im_rcol_vec.x * 255, im_rcol_vec.y * 255, im_rcol_vec.z * 255, im_rcol_vec.w * 255);

    // draw a bg for the rarity header.
    if (selected) {
      // card header background.
      // draw_list->AddRectFilled({ selection_tl.x, selection_tl.y }, { selection_br.x, top_y }, im_rcol, 0);
      // const auto header_bg_tl = ImVec2(card_ui_tl.x + header_pad, card_ui_tl.y + header_pad);
      // const auto header_bg_br = ImVec2(card_ui_tl.x + card_ui_wh.x - header_pad, card_ui_tl.y + header_pad +
      // header_y_size); const auto header_bg_wh = calc_wh(header_bg_tl, header_bg_br);
      // draw_list->AddRectFilled(header_bg_tl, header_bg_br, im_rcol, 0);

      // draw the card rarity.
      // const auto rarity_tl = ImVec2(header_bg_tl.x, header_bg_br.y);
      // const auto rarity_br = ImVec2(header_bg_br.x, header_bg_br.y + f->FontSize);
      // const auto rarity_wh = calc_wh(rarity_tl, rarity_br);
      // const auto rarity_txt_center = center_text(f, data.rarity_txt, calc_center(rarity_tl, rarity_wh));
      // draw_list->AddText(f, f->FontSize, rarity_txt_center, im_rcol, data.rarity_txt.c_str());
    }

    const auto box_tl = ImVec2{ first_x, center_y };
    const auto box_br = ImVec2{ box_tl.x + icon_size, box_tl.y + icon_size };
    const auto box_wh = box_br - box_tl;
    // draw_list->AddRect(box_tl, box_br, IM_COL32(255, 0, 0, 255));

    std::string header_text = "Upgrade!";
    std::string desc_text = "";

    // if only one stat, set the header
    if (result.stats.size() == 1) {
      const auto& s = result.stats[0];
      const std::string stat = s.stat;
      const std::string type = s.type;
      const float value = s.value;
      const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(stat).value();

      // flavour text for the header
      const auto [amount, type_str] = get_stat_from_stat_table(r, rarity, stat_enum);
      const UpgradeRollResult roll_res{ .rarity = rarity, .stats = { Stat{ .stat = stat } } };
      header_text = upg_name_c.stat_to_name_map.at(roll_res);
    }

    // draw selected stat header
    if (selected) {
      const auto py = 4.0f;
      const auto text_wh = font->CalcTextSizeA(font_size, FLT_MAX, -1, header_text.c_str());
      const auto text_tl = ImVec2{ selection_tl.x + 0.5f * (selection_wh.x - text_wh.x), selection_tl.y + py };
      const auto text_next_wh = font->CalcTextSizeA(font_size, FLT_MAX, -1, rarity_str.c_str());
      const auto text_next_tl =
        ImVec2{ selection_tl.x + 0.5f * (selection_wh.x - text_next_wh.x), text_tl.y + text_wh.y + py };

      ImGui::PushFont(font, font_size);
      draw_list->AddText(text_tl, im_text_col, header_text.c_str());
      draw_list->AddText(text_next_tl, im_rcol, rarity_str.c_str());
      ImGui::PopFont();
    }

    SelectableButtonDef def{
      .display_str = state_c.cells[i]->name,
      .imgui_hash = "##aquire_" + header_text + "_" + rarity_str,

      .size = box_wh,
      .input = do_act,
      .cell = state_c.cells[i],
      .active_cell = state_c.active,

      .font = font,
      .font_size = font_size,

      // hide the buttons (display handled elsewhere)
      .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    ImGui::SetCursorPos(box_tl);
    if (selectable_button(r, def)) {
      state_c.active->action();
      return;
    }

    // draw the background
    draw_list->AddRectFilled(box_tl, box_br, im_window_bg_col);
    draw_list->AddRect(box_tl, box_br, selected ? im_greenish : im_white);

    // add text "A", "B", "C"
    const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
    const auto text_pos = box_tl + ImVec2{ 0.5f, 0.5f } * (box_wh - TEXT_SIZE);
    if (i == 0)
      draw_list->AddText(font, font_size, text_pos, im_text_col, "A");
    else if (i == 1)
      draw_list->AddText(font, font_size, text_pos, im_text_col, "B");
    else if (i == 2)
      draw_list->AddText(font, font_size, text_pos, im_text_col, "C");

    // draw a pointer to the active selection
    if (selected) {
      auto& cursor_c = ui_c.ui_cursors[player_idx];
      draw_cursor(r, cursor_c, box_tl, dt);
    }

    first_x += icon_size + padding_x;
  }
};

void
draw_stats(entt::registry& r,
           ImVec2 stats_tl,
           ImVec2 stats_br,
           entt::entity player_e,
           const UIState& state_c,
           const UpgradeResultsComponent* upgrades_c,
           ImFont* font,
           const float font_size)
{
  auto* draw_list = ImGui::GetWindowDrawList();
  const auto& ri_c = SINGLE_RendererInfo::instance;
  const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom")->id;
  const auto custom_im_id = (ImTextureID)(void*)(intptr_t)custom_tex_id;

  const auto stats = get_display_stats(r);
  const auto max_width = calculate_width(stats);
  const auto icon_size = ImVec2{ 16, 16 };

  if (upgrades_c->results.empty())
    return;

  // this contains the upgrade data.
  const auto cell_it = std::find(state_c.cells.begin(), state_c.cells.end(), state_c.active);
  const auto cell_idx = static_cast<int>(cell_it - state_c.cells.begin());
  const std::vector<UpgradeRollResult> upgrades_vec = { upgrades_c->results.begin(), upgrades_c->results.end() };
  const auto result = upgrades_vec[cell_idx];
  const auto& upg_stats = result.stats;
  const auto& upg_traits = result.traits;
  const auto& upg_weapons = result.weapons;
  const auto weapons_e = get_weapons(r, player_e);

  auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const auto line_size = 16;
  int start_y = (int)stats_tl.y + 5; // add Xpx worth of padding
  const int key_x = stats_tl.x + icon_size.x + 5.0f;

  for (int i = 0; i < (int)stats.size(); i++) {
    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name(stat_enum));

    // stats to skip...
    if (stat_enum == UpgradeableStat::ACTOR_STAMINA)
      continue;

    // skip various stats on weapon types
    if (!upg_weapons.empty()) {
      const auto wep_e = upg_weapons[0];
      const auto& wep_data = r.get<const Weapon_OnDiskData>(wep_e);
      const auto wep_type = wep_data.type_as_enum;

      // skip the BULLET_ stats if you're not a PROJECTILE or DEPLOY weapon.
      const bool is_bullet_stat = stat_str.find("BULLET_") != std::string::npos;
      const auto wt = std::vector<WEAPON_TYPE>{ WEAPON_TYPE::PROJECTILE, WEAPON_TYPE::DEPLOY };
      if (is_bullet_stat && std::find(wt.begin(), wt.end(), wep_type) == wt.end())
        continue;

      // skip the AREA_ stats if you're not an AREA weapon.
      const bool is_area_stat = stat_str.find("AREA_") != std::string::npos;
      if (is_area_stat && wep_data.damage_as_enum != WEAPON_DAMAGE::FIRE)
        continue;
    }

    // value
    std::string val_str = get_val_str_from_stat_enum(r, player_e, upg_weapons, stat_enum);
    if (val_str == "N/A")
      continue;

    // Display weapon info (before bounce stat)
    bool display_weapon_info = magic_enum::enum_value<UpgradeableStat>(i) == UpgradeableStat::BULLET_BOUNCE;
    display_weapon_info &= !upg_weapons.empty();
    if (display_weapon_info) {
      const auto wep_e = upg_weapons[0];

      // display weapon (key)
      draw_list->AddText({ (float)key_x, (float)start_y }, im_text_col, "WEAPON");

      // display weapon name (val)
      const auto& weapon_data_c = r.get<const Weapon_OnDiskData>(wep_e);
      const auto weapon_name = std::format("{}", weapon_data_c.name);
      draw_list->AddText({ key_x + max_width + 5.0f, (float)start_y }, im_text_col, weapon_name.c_str());

      // display hardpoint idx (key)
      start_y += line_size;
      draw_list->AddText({ (float)key_x, (float)start_y }, im_text_col, "HARDPOINT");

      // display hardpoint idx (val)
      const auto it = std::find(weapons_e.begin(), weapons_e.end(), wep_e);
      const auto idx = static_cast<int>(it - weapons_e.begin());
      const auto idx_str = std::format("{}", idx);
      draw_list->AddText({ key_x + max_width + 5.0f, (float)start_y }, im_text_col, idx_str.c_str());

      // display current weapon level (key)
      start_y += line_size;
      draw_list->AddText({ (float)key_x, (float)start_y }, im_text_col, "LEVEL");

      // display current weapon level (val)
      const auto& wep_c = r.get<WeaponLevelComponent>(wep_e);
      const auto wep_lv_str = std::format("{}", wep_c.level);
      draw_list->AddText({ key_x + max_width + 5.0f, (float)start_y }, im_text_col, wep_lv_str.c_str());

      // assuming we're here, show a +1 to level because this would upgrade the weapon.
      draw_list->AddText({ key_x + max_width + 50.0f, (float)start_y }, im_greenish, "+1"s.c_str());

      start_y += line_size;
    }

    // some stats need spacers
    // TODO: remove string comparison
    if (stats[i].key == "BOUNCE")
      start_y += line_size;
    if (stats[i].key == "PROJECTILES")
      start_y += line_size;
    if (stats[i].key == "BEAMS_PER_WEAPON")
      start_y += line_size;

    const auto icon_tl = ImVec2{ stats_tl.x, (float)start_y };
    const auto icon_br = ImVec2{ stats_tl.x + icon_size.x, icon_tl.y + icon_size.y };
    const auto icon_wh = icon_br - icon_tl;
    const auto key_pos = ImVec2{ (float)key_x, (float)start_y };
    const auto val_pos = ImVec2{ key_x + max_width + 5.0f, (float)start_y };

    // icon
    const auto icon_key = "ICON_" + stat_str + "_CENTERED";
    auto [image_icon_tl, image_icon_br] = convert_sprite_to_uv(r, icon_key);
    draw_list->AddImage(custom_im_id, icon_tl, icon_br, image_icon_tl, image_icon_br);

    // key
    draw_list->AddText(key_pos, im_text_col, stats[i].key.c_str());

    // display your current stat value
    draw_list->AddText(val_pos, im_text_col, val_str.c_str());

    // display the new stat value (i.e. how much the upgrade will change it by)
    const auto is_stat = [&](const Stat& other) { return other.stat == stat_str; };
    const auto find_stat = std::find_if(result.stats.begin(), result.stats.end(), is_stat);
    if (find_stat != result.stats.end()) {
      const auto& stat = *find_stat;
      auto stat_amount = std::format("+{:0.1f}", stat.value);
      if (stat.type == "stat_percent_increase")
        stat_amount += "%";
      draw_list->AddText({ key_x + max_width + 50.0f, (float)start_y }, im_greenish, stat_amount.c_str());
    }

    // move vertically
    start_y += line_size;
  }

  // list the weapon behaviours.
  if (!upg_weapons.empty()) {
    // add a separator
    start_y += line_size;
    draw_list->AddText({ (float)key_x, (float)start_y }, im_text_col, "Overclocks (Lv 4, 8, 12)");

    const auto wep_e = upg_weapons[0];
    const auto& weapon_key = r.get<ItemKey>(wep_e);
    const auto weapon_upgrades_data = get_upgrades_from_weapon_key(r, weapon_key.key);

    const auto aquired_upg = get_aquired_upgrades(r, weapon_upgrades_data, wep_e);
    const auto unaquired_upg = get_unaquired_upgrades(r, weapon_upgrades_data, wep_e);

    // aquired upgrades
    for (const auto& u_key : aquired_upg) {
      start_y += line_size;
      const auto dis_str = get_display_key_from_upgrade_key(r, u_key);
      draw_list->AddText({ (float)key_x, (float)start_y }, im_text_col, dis_str.c_str());
    }

    // which behaviour are we upgrading
    std::optional<WeaponBehaviour> behaviour = std::nullopt;
    if (!upg_traits.empty())
      behaviour = upg_traits[0];

    // display unaquired upgrades
    for (const std::string& u_key : unaquired_upg) {
      start_y += line_size;

      const auto wb_key = get_wb_key_from_upgrade_key(r, u_key);
      const auto dis_str = get_display_key_from_upgrade_key(r, u_key);

      // whichever behaviour is selected, highlight it
      auto col = im_inactive_col;
      if (behaviour.has_value()) {
        auto b_enum = magic_enum::enum_cast<WeaponBehaviour>(wb_key).value();
        if (behaviour.value() == b_enum)
          col = im_greenish;
      }

      draw_list->AddText({ (float)key_x, (float)start_y }, col, dis_str.c_str());
    }
  }

  // if (result.traits.empty()) {
  //   start_y += line_size;
  //   draw_list->AddText({ key_x, start_y }, im_text_col, "NONE. Get @ Lv 4, 8, 12");
  // }
};

void
draw_confirm_bar(entt::registry& r,
                 ImVec2 tl,
                 ImVec2 br,
                 const int player_idx,
                 const CardUIUpgradeComponent& card_ui_c,
                 ImFont* font,
                 const float font_size)
{
  auto* draw_list = ImGui::GetWindowDrawList();
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_ui_e, steam_ui_c);

  const auto purchasebar_tl = ImVec2{ tl.x + 5.0f, br.y - 25.0f };
  const auto purchasebar_br = ImVec2{ br.x - 5.0f, br.y - 5.0f };
  const auto purchasebar_wh = purchasebar_br - purchasebar_tl;
  // draw_list->AddRect(purchasebar_tl, purchasebar_br, im_player_col);

  auto my_player_col = default_player_colours[player_idx];
  auto my_player_col_active = my_player_col;
  auto my_player_col_inactive = my_player_col;
  my_player_col_inactive.a = 0.25f * 255;
  const auto im_player_col_active = convert_my_to_im(my_player_col_active);
  const auto im_player_col_inactive = convert_my_to_im(my_player_col_inactive);

  const float bar_rounding = 0.0f;

  const auto draw_bar = [&](const ImVec2 bar_tl, const ImVec2 bar_br, const float percent) {
    const ImVec2 bar_wh = bar_br - bar_tl;

    // draw a box around the bar
    draw_list->AddRect(bar_tl, bar_br, im_player_col_active, bar_rounding, ImDrawFlags_RoundCornersAll, 1);

    // bar bg
    draw_list->AddRectFilled(bar_tl, bar_br, im_player_col_inactive, bar_rounding, ImDrawFlags_RoundCornersAll);

    // bar fg
    float x = bar_tl.x + percent * bar_wh.x;
    const auto partial_bar_br = ImVec2(x, bar_br.y);
    ImU32 col_l = im_player_col_active;
    ImU32 col_r = im_player_col_inactive;
    draw_list->AddRectFilledMultiColor(bar_tl, partial_bar_br, col_r, col_l, col_l, col_r);
  };
  draw_bar(purchasebar_tl, purchasebar_br, card_ui_c.time_to_confirm_cur / card_ui_c.time_to_confirm_max);

  // draw some text on the inside of the bar
  const auto center_pos = ImVec2{ purchasebar_tl.x + purchasebar_wh.x * 0.5f, purchasebar_tl.y + purchasebar_wh.y * 0.5f };
  const auto handle = steam_ui_c.handles[player_idx];
  const auto ready_text = std::format("Hold {} to select", get_confirm_button_str(r, handle));
  const auto ready_text_pos = center_text(font, font_size, ready_text, center_pos);
  ImGui::SetCursorPos(ready_text_pos);
  ImGui::Text("%s", ready_text.c_str());
};

void
update_ui_survive_upgrade_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

#if defined(_DEBUG)
  const auto& scene_c = SINGLE_CurrentScene::instance;
  if (scene_c.s == Scene::menu) {
    gesert_component<SINGLE_XpComponent>(r);
    gesert_component<SINGLE_LevelUpUI>(r);
    gesert_component<SINGLE_PersistentUpgrades>(r);
    static bool init = false;
    if (!init) {
      for (int i = 0; i < 4; i++) {
        // create a fake player
        spawn_player(r, "actor_player", i, "dinghy", "weapon_deck_cannon", { 0, 0 });
      }
      init = true;
    }
  }
#endif

  const auto& ri_c = SINGLE_RendererInfo::instance;
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_LevelUpUI, r, ui_e, ui_c);
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, up_e, up_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_ui_e, steam_ui_c);
  auto& evts_c = SINGLE_Events::instance;

#if defined(_DEBUG)

  // Cheats..!! CHEATSS!!! CHEEEATTTSSSSSSS!!!!!!!
  {
    auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
    // auto cheat_levelup_state = gesert_menubar_state(menu_c, "Cheat LevelUp");

    // if (cheat_levelup_state.enabled) {
    if (true) {

      ImGui::SetNextWindowPos(ImVec2{ (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y },
                              ImGuiCond_Always,
                              { 1.0f, 1.0f });
      ImGui::SetNextWindowSize({ 200, 100 });

      ImGuiWindowFlags flags = 0;
      flags |= ImGuiWindowFlags_NoDecoration;
      flags |= ImGuiWindowFlags_NoMove;
      flags |= ImGuiWindowFlags_NoSavedSettings;

      ImGui::Begin("CheatLevelUp", nullptr, flags);

      if (ImGui::Button("LevelUp") && r.view<UpgradeResultsComponent>().size() == 0)
        sxp_c.xp += sxp_c.xp_for_next_level;

      for (int i = 0; i < 4; i++) {
        const auto player_e = get_player_e_from_idx(r, i);
        const auto upgrades = find<UpgradeResultsComponent>(r, player_e);
        ImGui::Text("upgrades_c: %zu", upgrades.size());
      }

      ImGui::End();
    }
  }

#endif

  // check the probabilities are mathing to 100%
  static_assert(sum_array_values() == 100);

#if defined(_DEBUG)
  const int max_num_players = 4;
  const int num_active_players = 4;
#else
  const int max_num_players = 4;
  const auto num_active_players = glm::max(1, (int)non_zero_handles(steam_ui_c.handles).size());
#endif

  const auto text_col = ImVec4(0.64f, 0.64f, 0.64f, 1.0f);

  if (ui_c.ui_states.empty())
    ui_c.ui_states.resize(max_num_players);
  ui_c.ui_cursors.resize(num_active_players);

  if (sxp_c.xp >= sxp_c.xp_for_next_level) {
    // consume xp
    sxp_c.xp = 0;
    sxp_c.level++;
    sxp_c.xp_for_next_level += 5; // 5 harder every time

    SDL_Log("Levelup");
    generate_upgrades_for_players(r, ui_c);
    populate_ui_based_on_upgrades(r, ui_c);
  }

  // dont show upgrade ui
  ui_c.open = false;
  if (!is_choosing_upgrade(r))
    return;
  ui_c.open = true;

  const float upg_header_height = 75.0f;
  const float line_size = 40.0f;
  const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom")->id;
  const auto custom_im_id = (ImTextureID)(void*)(intptr_t)custom_tex_id;

  auto* head_f = get_fingerpaint_font(r);
  auto* font = get_inter_font(r);
  auto font_size = (float)FontSizes::SIZE_13;
  auto head_f_font_size = (float)FontSizes::SIZE_20;

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const float window_x_size = ri_c.viewport_size_render_at.x;
  const float window_y_size = ri_c.viewport_size_render_at.y;
  const auto set_window_size = ImVec2{ window_x_size, window_y_size };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

  imgui_begin("UpgradeUI");
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();
  const ImVec2 player_wh = { ui_wh.x / (float)max_num_players, ui_wh.y };

  auto* draw_list = ImGui::GetWindowDrawList();

  const auto pivot = 0.5f;
  const auto card_pad_x = 5.0f;
  const auto card_width = 300.0f;
  const auto card_height = 400.0f * (16.0f / 9.0f);
  const auto center_x = ui_tl.x + 0.5f * ui_wh.x;
  const auto center_y = ui_tl.y + ui_wh.y * 0.5f;

  auto first_tl_x = center_x;
  first_tl_x -= max_num_players * (0.5f * card_width);
  const auto ui_move_horizontally = [&]() { first_tl_x += card_width; };

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    const auto player_e = get_player_e_from_idx(r, player_idx);
    if (player_e == entt::null) {
      ui_move_horizontally();
      continue;
    }

    const auto upgrades = find<UpgradeResultsComponent>(r, player_e);
    if (upgrades.empty()) {
      ui_move_horizontally();
      continue; // this player isnt upgrading
    }

    ImGui::PushID((uint32_t)player_e);

    // process the first upgrade results
    const std::pair<entt::entity, UpgradeResultsComponent*>& upgs_pair = upgrades[0];
    const auto* upgrades_c = upgs_pair.second;

    // update input
    auto& state_c = ui_c.ui_states[player_idx];
    state_c.actions.clear();
    const auto input = r.get<const InputComponent>(player_e);
    process_input_for_ui(r, state_c, input);

    // make it so the user holds the confirm button
    bool do_act = false;
    auto& card_ui_c = r.get_or_emplace<CardUIUpgradeComponent>(player_e);
    {
      const bool do_act_down = has(input.button_s, ActionStateEnum::DOWN);
      const bool do_act_held = has(input.button_s, ActionStateEnum::HELD);
      const bool do_act_release = has(input.button_s, ActionStateEnum::RELEASE);

      // dont buy multiple upgrades if you hold a button
      if (do_act_down)
        card_ui_c.released_since_action = true;
      if (do_act_release)
        card_ui_c.released_since_action = true;

      // reset timer
      if (!do_act_held)
        card_ui_c.time_to_confirm_cur = 0.0f;

      // if button held, increase timer
      if (do_act_held && card_ui_c.released_since_action)
        card_ui_c.time_to_confirm_cur += dt;

      // if timer > threshold, do the act.
      if (card_ui_c.time_to_confirm_cur >= card_ui_c.time_to_confirm_max && card_ui_c.released_since_action) {
        do_act = true;
        card_ui_c.released_since_action = false;
        card_ui_c.time_to_confirm_cur = 0.0f;
      }
    }

    auto* f = font; // body font
    const auto im_player_col = default_player_colours_im[player_idx];

    const auto card_center_x = first_tl_x + 0.5f * card_width;
    const auto card_tl = ImVec2{ card_center_x - 0.5f * card_width + card_pad_x, center_y - (card_height * pivot) };
    const auto card_br = ImVec2{ card_center_x + 0.5f * card_width - card_pad_x, card_tl.y + card_height };

    // const auto clamped_tl = player_ui_tl;
    // const auto clamped_br = player_ui_br;
    const auto clamped_tl = card_tl;
    const auto clamped_br = card_br;
    const auto clamped_wh = clamped_br - clamped_tl;
    // draw_list->AddRect(clamped_tl, clamped_br, IM_COL32(255, 0, 0, 255));

    // work out the selection box tl & br
    const auto stats_y = upg_header_height + 60.0f;
    const auto padding_x = 10.0f;
    const auto selection_tl = ImVec2{ clamped_tl.x + padding_x, clamped_tl.y + head_f_font_size + 50.0f };
    const auto selection_br = ImVec2{ clamped_tl.x + clamped_wh.x - padding_x, clamped_tl.y + stats_y + 50.0f };

    // work out the stats box tl & br
    const auto stats_tl = ImVec2{ clamped_tl.x + padding_x, selection_br.y };
    const auto stats_br = ImVec2{ clamped_tl.x + clamped_wh.x - padding_x, clamped_br.y - 50.0f };

    // draw a background for the upgrade box.
    draw_list->AddRectFilled(selection_tl, stats_br, im_window_bg_col);
    draw_list->AddRect(selection_tl, stats_br, im_player_col, 0.0f, 0, 1.0f);

    // add upgrade header for column
    const auto upg_text = "Choose your Upgrade";
    const auto upg_pos = ImVec2{ clamped_tl.x + 0.5f * clamped_wh.x, selection_tl.y - head_f_font_size }; // top center
    const auto upg_center = center_text(head_f, head_f_font_size, upg_text, upg_pos, { 0.5f, 0.0f });
    draw_list->AddText(head_f, head_f_font_size, upg_center, im_player_col, upg_text);

    // some separator lines
    const auto s_y = upg_pos.y + 0.5f * head_f_font_size;
    const auto l0_p1 = ImVec2{ selection_tl.x, s_y };
    const auto l0_p2 = ImVec2{ selection_tl.x + line_size, s_y };
    draw_list->AddLine(l0_p1, l0_p2, im_player_col);
    const auto l1_p1 = ImVec2{ selection_br.x - line_size, s_y };
    const auto l1_p2 = ImVec2{ selection_br.x, s_y };
    draw_list->AddLine(l1_p1, l1_p2, im_player_col);

    // Draw the upgrades in a grid
    // draw_list->AddRect(selection_tl, selection_br, im_player_col);
    draw_upgrade_selections_in_grid(
      r, selection_tl, selection_br, state_c, ui_c, upgrades_c, player_idx, do_act, dt, font, font_size);

    if (do_act) {
      ui_move_horizontally();
      ImGui::PopID();
      continue;
    }

    // draw the confirm bar#
    ImGui::PushFont(font, font_size);
    draw_confirm_bar(r, selection_tl, selection_br, player_idx, card_ui_c, font, font_size);
    ImGui::PopFont();

    // Draw the stats
    // draw_list->AddRect(stats_tl, stats_br, im_player_col);
    draw_stats(r, stats_tl, stats_br, player_e, state_c, upgrades_c, font, font_size);

    ui_move_horizontally();
    ImGui::PopID();
  }

  ImGui::End();
  ImGui::PopStyleVar(3);
};

} // namespace game2d

/*
ImGui::Begin("DebugPlayerInputs");
{
const auto& controller_ui = get_first_component<SINGLE_SteamControllerGameState>(r);
// for (const auto handle : controller_ui.handles)
//   ImGui::Text("Handle: %zu", (uint64)handle);

for (int i = 0; const auto& [e, player_c, input_c] : r.view<const PlayerComponent, const InputComponent>().each()) {
  ImGui::PushID((uint32_t)e);

  ImGui::Text("p%i lx %f ly %f", player_c.idx, input_c.lx, input_c.ly);

  if (r.all_of<PlayerBoatComponent>(e)) {
    ImGui::SameLine();
    ImGui::Text("(boat)");

    if (!r.all_of<MovementDirectComponent>(e))
      r.emplace<MovementDirectComponent>(e);
  }

  if (r.all_of<MovementIslandComponent>(e)) {
    ImGui::SameLine();
    ImGui::Text("(islander)");
  }
  if (r.all_of<KeyboardComponent>(e)) {
    ImGui::SameLine();
    ImGui::Text("(keyboard)");
  }
  if (r.all_of<SteamControllerComponent>(e)) {
    ImGui::SameLine();
    auto& steam_c = r.get<SteamControllerComponent>(e);
    ImGui::Text("(controller) handles: %i", (int)steam_c.handles.size());
  }

  ImGui::SameLine();
  if (ImGui::Button("Control")) {
    const auto view0 = r.view<SteamControllerComponent>(entt::exclude<Persistent>);
    for (auto entity : view0)
      r.remove<SteamControllerComponent>(entity);
    const auto view1 = r.view<KeyboardComponent>();
    for (auto entity : view1)
      r.remove<KeyboardComponent>(entity);

    // note:assign all handles to the player.
    // (i.e. all controllers)
    // only use for debug
    SteamControllerComponent steam_c;
    steam_c.handles = non_zero_handles(controller_ui.handles);
    if (steam_c.!handles.empty())
      r.emplace<SteamControllerComponent>(e, steam_c);
    r.emplace<KeyboardComponent>(e);
  }

  ImGui::Separator();
  ImGui::PopID();
  i++;
}
}
ImGui::End();
*/