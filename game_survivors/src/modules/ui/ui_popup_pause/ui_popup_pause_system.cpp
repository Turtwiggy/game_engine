#include "pch.hpp"

#include "modules/ui/ui_popup_pause/ui_popup_pause_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/std/vector/helpers.hpp"
#include "game_state.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_popup_pause/ui_popup_pause_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "ui_popup_pause_components.hpp"

namespace game2d {

void
update_ui_popup_pause_system(engine::SINGLE_Application& app, entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_PauseMenuState, r, ui_e, ui_c);
  const auto ui_scale = get_first_component<SINGLE_UIScaling>(r).scaling;

  // const std::vector<Scene> scene_to_show_pause_menu{
  //   Scene::survive,
  //   Scene::develop_snake,
  //   Scene::develop_islands,
  // };

  const std::vector<Scene> scene_not_to_show_pause_menu{
    Scene::splashscreen, Scene::pressanykey, Scene::menu, Scene::select_modifiers, Scene::select_ships,
  };
  const auto& scene_c = SINGLE_CurrentScene::instance;
  if (has(scene_not_to_show_pause_menu, scene_c.s))
    return;

  // TEMPORARY: input to generate open/close events
  {
    GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)
    if (get_key_down(input, SDL_SCANCODE_ESCAPE))
      create_empty<RequestToShowPauseMenu>(r);
  }

  ui_c.update<RequestToShowPauseMenu>(r);

  const bool open = ui_c.open;
  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (open)
    state.state = state.state == GameState::RUNNING ? GameState::PAUSED : state.state;
  if (!open)
    state.state = state.state == GameState::PAUSED ? GameState::RUNNING : state.state;
  if (!open)
    return;

  if (!ui_c.init)
    ui_c.do_init(r);

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  const auto center = ImVec2{
    (float)ri.viewport_pos.x + viewport_size_half.x,
    (float)ri.viewport_pos.y + viewport_size_half.y,
  };
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowBgAlpha(0.0f);

  const auto font_size = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  // flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);
  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const bool do_act = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();

  ImGui::Begin("Paused", NULL, flags);

  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  const ImVec2 size = { 120.0f * ui_scale, 40.0f * ui_scale };

  for (int i = 0; i < (int)ui_c.state.cells.size(); i++) {
    if (i > 0)
      ImGui::NewLine();
    auto& cell = ui_c.state.cells[i];

    auto a_def = SelectableButtonDef{
      .display_str = cell->name,
      .imgui_hash = "##" + cell->name,
      .size = size,
      .input = do_act,
      .cell = cell,
      .active_cell = ui_c.state.active,
      .font = font,
      .font_size = font_size,
    };

    if (selectable_button(r, a_def))
      cell->action();
  }

  ImGui::End();
  ImGui::PopStyleVar(5);
};

} // namespace game2d