#include "pch.hpp"

#include "scene_select_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/actor_weapon/weapon_components.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "scene_select_components.hpp"

namespace game2d {

void
update_countdown(SINGLE_SelectSceneData& data_c, bool everyone_confirmed, float dt)
{
  // Start a countdown..
  if (everyone_confirmed)
    data_c.countdown -= dt;
  else
    data_c.countdown = data_c.countdown_max;
};

void
update_countdown_ui(entt::registry& r, const SINGLE_SelectSceneData& data_c)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto x_pos = 0.5f;
  const auto y_pos = 0.75f;
  const auto hmm = ImVec2{ ri.viewport_size_render_at.x * x_pos, ri.viewport_size_render_at.y * y_pos };
  const auto pos = ImVec2{ ri.viewport_pos.x + hmm.x, ri.viewport_pos.y + hmm.y };
  const auto size = ImVec2{ 200, 200 };
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(size);

  ImGuiWindowFlags countdown_flags = 0;
  countdown_flags |= ImGuiWindowFlags_NoDecoration;
  countdown_flags |= ImGuiWindowFlags_NoCollapse;
  countdown_flags |= ImGuiWindowFlags_NoTitleBar;
  countdown_flags |= ImGuiWindowFlags_NoBackground;

  ImGuiIO& io = ImGui::GetIO();
  ImGui::PushFont(io.Fonts->Fonts[2]); // Use the larger font (index 1)
  ImGui::Begin("Countdown", NULL, countdown_flags);
  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  const auto str = std::format("{}", ((int)data_c.countdown) + 1);
  const auto text_c_str = str.c_str();
  const auto text_c_str_len = ImGui::CalcTextSize(text_c_str);
  ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (text_c_str_len.x * 0.5));
  ImGui::SetCursorPosY(ui_tl.y + (ui_wh.y * 0.5) - (text_c_str_len.y * 0.5));
  ImGui::Text("%s", text_c_str);

  ImGui::End();
  ImGui::PopFont();
};

void
update_countdown_to_next_scene(entt::registry& r,
                               const SINGLE_SelectSceneData& data_c,
                               const std::vector<ShipHullData>& sorted_hulls)
{
  if (data_c.countdown > 0.0f)
    return;

  SelectSceneToSurviveScene data;
  data.chosen_boats = data_c.player_choice_state;
  create_persistent<SelectSceneToSurviveScene>(r, data);

  move_to_scene_start(r, Scene::survive);
};

void
update_input_for_select_ui(entt::registry& r, SINGLE_SelectSceneData& ui_c)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);

  const int joined_players = non_zero_handles(steam_state_c.handles).size();
  for (int i = 0; i < 4; i++) {

    // 4 copies of the ui-state. one per player.
    auto& ui_state_c = ui_c.player_ui_state[i];
    if (!ui_state_c.init) {
      ui_state_c.rows.push_back({ .col_name = "Select Weapon", .action = []() {} });
      ui_state_c.rows.push_back({ .col_name = "Select Hull", .action = []() {} });
      ui_state_c.init = true;
    }

    // process the input for that ui-state.
    ui_state_c.new_actions.clear();

    if (i < joined_players)
      process_input_for_ui(r, ui_state_c, steam_state_c.handles[i]);
  }
};

void
draw_player_select_box(entt::registry& r,
                       SINGLE_SelectSceneData& ui_c,
                       const ImVec2 tl,
                       const ImVec2 wh,
                       const int player_idx)
{
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);

  const int num_active_players = non_zero_handles(steam_state_c.handles).size();
  const bool player_is_joined = player_idx < num_active_players;

  if (!player_is_joined) {
    const auto text_str = "Join!";
    const auto text_str_len = ImGui::CalcTextSize(text_str);
    const auto text_size = ImGui::CalcTextSize(text_str);
    const auto text_pos = ImVec2{
      tl.x + 0.5f * (wh.x - text_size.x),
      tl.y + 0.5f * (wh.y - text_size.y),
    };
    ImGui::GetWindowDrawList()->AddText(text_pos, IM_COL32(255, 255, 255, 255), text_str);
    return;
  }

  // const auto button_size_w = 20.0f;
  // const auto button_size_half = button_size_w * 0.5f;
  // const auto button_size = ImVec2{ button_size_w, button_size_w };
  // const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  // const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));

  const auto text_size = ImGui::CalcTextSize("anything");
  auto& state_c = ui_c.player_ui_state[player_idx];
  auto& game_state_c = ui_c.player_choice_state[player_idx];

  auto last_xy = tl;
  last_xy.y += 0.5f * (wh.y - text_size.y);
  last_xy.y -= 0.5f * state_c.rows.size() * text_size.y; // center

  for (int row_i = 0; row_i < (int)state_c.rows.size(); row_i++) {
    auto& row = state_c.rows[row_i];
    auto& col_index = row.col_index;
    const auto& col_name = row.col_name;

    const bool selected = state_c.current_row_index == row_i;
    const bool is_weapon = row_i == 0;
    const bool is_hull = row_i == 1;

    const auto text = row.col_name;
    std::string label = "";

    // prepend (x) to label
    // if (selected)
    //   label += "(x) ";
    label += std::format("{}", text);

    // append weapon name to label
    if (is_weapon) {
      auto& idx = col_index;

      // convert index to valid index
      const auto& weapons = weapons_c.weapons;
      idx = idx < 0 ? (int)weapons.size() - 1 : idx;
      idx %= weapons.size();

      const auto& weapon = weapons[idx];
      label += ": " + weapon.name;

      // update state? note: .key not .name
      game_state_c.player_gun = weapon.key;
      SDL_Log("Updated player gun choice: %s", game_state_c.player_gun.c_str());
    }

    // append hull name to label
    if (is_hull) {
      auto& idx = col_index;

      // convert index to valid index
      const auto& hulls = hulls_c.hulls;
      idx = idx < 0 ? (int)hulls.size() - 1 : idx;
      idx %= hulls.size();

      const auto& hull = hulls[idx];
      label += ": " + hull.name;

      // update state? note: .key not .name
      game_state_c.player_boat = hull.key;
    }

    last_xy.y += text_size.y;
    const auto text_pos_l = ImVec2{
      tl.x,
      last_xy.y,
    };

    auto colour = IM_COL32(255, 255, 255, 126);
    if (selected)
      colour = IM_COL32(255, 255, 255, 255);

    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddText(last_xy, colour, label.c_str());

    // Left Arrow
    // ImGui::SetCursorScreenPos(text_pos_l);
    // ImVec2 uv_tl{ 0.0f, 0.0f };
    // ImVec2 uv_br{ 1.0f, 1.0f };
    // std::string l_label = "##left"s + std::to_string(row_i);
    // std::tie(uv_tl, uv_br) = convert_sprite_to_uv(r, "ARROW_LEFT");
    // ImGui::ImageButton(l_label.c_str(), im_id, button_size, uv_tl, uv_br);

    // hull description
    if (is_hull) {
      const auto idx = col_index;
      const auto& hulls = hulls_c.hulls;
      const auto& hull = hulls[idx];
      const auto desc = hull.desc;
      last_xy.y += text_size.y;
      ImGui::GetWindowDrawList()->AddText(last_xy, colour, desc.c_str());
    }

    // weapon description
    if (is_weapon) {
      const auto idx = col_index;
      const auto& weapons = weapons_c.weapons;
      const auto& weapon = weapons[idx];
      const auto desc = weapon.desc;
      last_xy.y += text_size.y;
      ImGui::GetWindowDrawList()->AddText(last_xy, colour, desc.c_str());
    }

    last_xy.y += text_size.y;
  }

  last_xy.y += text_size.y;
  last_xy.y += text_size.y;
  const auto text = std::format("Ready: {}", game_state_c.confirmed);
  ImGui::GetWindowDrawList()->AddText(last_xy, IM_COL32(255, 255, 255, 255), text.c_str());
}

// Display all the hulls,
// and the player can select one
void
update_ui_scene_select_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input_c)
  GET_FIRST_OR_RETURN(SINGLE_Hulls, r, hulls_e, hulls_c)
  GET_FIRST_OR_RETURN(SINGLE_Weapons, r, weapons_e, weapons_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)

  const int max_num_players = 4;
  const int num_active_players = non_zero_handles(steam_state_c.handles).size();

  if (ui_c.menu_to_select_scene_buffer_frame) {
    ui_c.menu_to_select_scene_buffer_frame = false;

    // init ui
    ui_c.player_ui_state.resize(max_num_players);
    ui_c.player_choice_state.resize(max_num_players);
    return;
  }

  update_input_for_select_ui(r, ui_c);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoDocking;

  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const auto set_window_size = ImVec2{ (float)ri_c.viewport_size_render_at.x - 0.1f * ri_c.viewport_size_render_at.x, 200 };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  ImGui::Begin("SelectShipUI", nullptr, flags);

  const ImVec2 window_pos = ImGui::GetWindowPos();
  const ImVec2 window_size = ImGui::GetWindowSize();
  const auto player_ui_w = window_size.x / max_num_players; // always /4
  const auto player_ui_h = window_size.y;
  auto player_ui_tl = ImVec2{ window_pos.x, window_pos.y };
  auto player_ui_br = ImVec2{ window_pos.x + player_ui_w, window_pos.y + player_ui_h };

  // center ui...
  // player_ui_tl.x += (window_size.x) * 0.5f - (num_active_players * player_ui_w * 0.5);
  // player_ui_br.x += (window_size.x) * 0.5f - (num_active_players * player_ui_w * 0.5);

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {

    // background
    const float inc = ((player_idx + 1) / 4.0f);
    const auto im_active_col = IM_COL32(0, 0, 255 * inc, 255);
    auto p_max = ImVec2{ player_ui_tl.x + player_ui_w, player_ui_tl.y + player_ui_h };
    ImGui::GetWindowDrawList()->AddRectFilled(player_ui_tl, p_max, im_active_col, 6);

    // draw the contents
    draw_player_select_box(r, ui_c, player_ui_tl, { player_ui_w, player_ui_h }, player_idx);

    // Input <=> Confirmed
    auto& state_c = ui_c.player_ui_state[player_idx];
    auto& game_state_c = ui_c.player_choice_state[player_idx];
    const auto& a = state_c.new_actions;
    const bool player_pressed_select = std::find(a.begin(), a.end(), UIAction::SELECT) != a.end();
    const bool player_pressed_back = std::find(a.begin(), a.end(), UIAction::BACK) != a.end();
    if (player_pressed_select)
      game_state_c.confirmed = true;
    if (player_pressed_back)
      game_state_c.confirmed = false;

    // move horizontally
    player_ui_tl.x += player_ui_w;
    player_ui_br.x += player_ui_w;
  }

  ImGui::End();
  ImGui::PopStyleVar(2);

  //
  // Show a countdown timer when all joined players are ready
  //

  const std::vector<HullChoice> c = { ui_c.player_choice_state.begin(),
                                      ui_c.player_choice_state.begin() + num_active_players };

  const auto confirmed = [](const HullChoice& choice) { return choice.confirmed; };
  const auto everyone_confirmed = std::all_of(c.begin(), c.end(), confirmed);
  const auto someone_confirmed = std::any_of(c.begin(), c.end(), confirmed);
  const auto noone_confirmed = std::none_of(c.begin(), c.end(), confirmed);

  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);

  update_countdown(ui_c, everyone_confirmed, dt);

  if (everyone_confirmed)
    update_countdown_ui(r, ui_c);

  update_countdown_to_next_scene(r, ui_c, hulls_c.hulls);

  ImGui::PopStyleVar();
}

} // namespace game2d