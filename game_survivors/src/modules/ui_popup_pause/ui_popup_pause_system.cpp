#include "pch.hpp"

#include "modules/ui_popup_pause/ui_popup_pause_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "game_state.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "ui_popup_pause_components.hpp"

namespace game2d {

void
update_ui_popup_pause_system(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_PauseMenuState, r, ui_e, ui_c);

  // only allow pause in survive scene
  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  if (scene.s != Scene::survive)
    return;

  // TEMPORARY: input to generate open/close events
  {
    GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)
    if (get_key_down(input, SDL_SCANCODE_ESCAPE))
      create_empty<RequestToShowPauseMenu>(r);
  }

  process_requests<RequestToShowPauseMenu>(r, [&ui_c](const auto& req) { ui_c.open = true; });

  const bool open = ui_c.open;
  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (open)
    state.state = state.state == GameState::RUNNING ? GameState::PAUSED : state.state;
  if (!open)
    state.state = state.state == GameState::PAUSED ? GameState::RUNNING : state.state;
  if (!open) {
    ui_c.state.current_row_index = 0; // reset ui state
    return;
  }

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  const auto center = ImVec2{
    (float)ri.viewport_pos.x + viewport_size_half.x,
    (float)ri.viewport_pos.y + viewport_size_half.y,
  };
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowBgAlpha(0.0f);

  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto font_size = (float)font_enum;
  auto* font = get_fingerpaint_font(r, font_enum);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  if (!ui_c.state.init) {

    auto resume_action = [&]() { ui_c.open = false; };
    auto options_action = [&]() {
      ui_c.open = false;
      create_empty<RequestToShowOptionsMenu>(r);
    };
    auto quit_to_menu_action = [&]() {
      ui_c.open = false;
      move_to_scene_start(r, Scene::menu);
    };
    auto quit_to_desktop_action = [&]() { app.running = false; };

    ui_c.state.rows.push_back(RowState{ .col_name = "Resume", .action = resume_action });
    ui_c.state.rows.push_back(RowState{ .col_name = "Options", .action = options_action });
    ui_c.state.rows.push_back(RowState{ .col_name = "Quit To Menu", .action = quit_to_menu_action });
    ui_c.state.rows.push_back(RowState{ .col_name = "Quit To Desktop", .action = quit_to_desktop_action });

    ui_c.state.init = true;
  }

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);
  process_input_for_ui_all_handles(r, ui_c.state);

  bool do_act =
    std::find(ui_c.state.actions.begin(), ui_c.state.actions.end(), UIAction::SELECT) != ui_c.state.actions.end();

  ImGui::Begin("Paused", NULL, flags);

  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  const ImVec2 size = { 120.0f, 40.0f };

  for (int i = 0; i < (int)ui_c.state.rows.size(); i++) {
    if (i > 0)
      ImGui::NewLine();
    auto& row = ui_c.state.rows[i];

    int col_idx = 0;

    auto a_def = SelectableButtonDef{
      .label = row.col_name,
      .size = size,
      .input = do_act,
      .my_row_index = i,
      .my_col_index = 0, // one col
      .ui_row_index = ui_c.state.current_row_index,
      .ui_col_index = col_idx, // one col
      .ui_col_active = true,   // one col
      .font = font,
    };

    if (selectable_button(r, a_def))
      row.action();
  }

  ImGui::End();
  ImGui::PopStyleVar(5);
};

} // namespace game2d