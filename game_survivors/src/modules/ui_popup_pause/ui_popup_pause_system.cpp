#include "modules/ui_popup_pause/ui_popup_pause_system.hpp"

#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_components.hpp"

#include "engine/entt/helpers.hpp"
#include "game_state.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "ui_popup_pause_components.hpp"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mixer.h>
#include <imgui.h>
#include <magic_enum.hpp>

namespace game2d {

void
update_ui_popup_pause_system(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)

  // TEMPORARY: input to generate open/close events
  {
    GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)
    if (get_key_down(input, SDL_SCANCODE_ESCAPE))
      create_empty<RequestToShowPauseMenu>(r);
  }

  static bool open = false;
  process_requests<RequestToShowPauseMenu>(r, []() { open = true; });

  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (open)
    state.state = state.state == GameState::RUNNING ? GameState::PAUSED : state.state;
  if (!open)
    state.state = state.state == GameState::PAUSED ? GameState::RUNNING : state.state;

  if (!open) {
    destroy_first<SINGLE_PauseMenuState>(r);
    return;
  }

  auto& ui_c = gesert_component<SINGLE_PauseMenuState>(r);

  const auto half_wh = ImVec2{ ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f };
  const auto center = ImVec2{ (float)ri.viewport_pos.x + half_wh.x, (float)ri.viewport_pos.y + half_wh.y };
  ImGui::SetNextWindowPos(center, ImGuiCond_Always, { 0.5f, 0.5f });

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiDockNodeFlags_PassthruCentralNode;

  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, pivot);
  ImGui::SetNextWindowBgAlpha(0.0f);

  // Note: although technically this is in a menu,
  // to avoid weird bugs between swapping action & get_button_down,
  // consdier the pause menu as a Game action set.
  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);

  process_input_for_ui(r, ui_c.state);
  bool& do_act = ui_c.state.do_action;
  int& selected = ui_c.state.selected;

  ImGui::Begin("Paused", NULL, flags);

  ImGui::Text("Selected: %i", selected);

  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  const ImVec2 size = { 120.0f, 40.0f };

  int index = 0;

  {
    auto def = SelectableButtonDef{
      .label = "Resume",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    ImGui::NewLine();
    if (selectable_button(def))
      open = false;
  }
  {
    auto def = SelectableButtonDef{
      .label = "Options",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    ImGui::NewLine();
    if (selectable_button(def)) {
      open = false;

      create_empty<RequestToShowOptionsMenu>(r);
    }
  }
  {
    auto def = SelectableButtonDef{
      .label = "Quit to Menu",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    ImGui::NewLine();
    if (selectable_button(def)) {
      move_to_scene_start(r, Scene::menu);

      open = false; // unpause this menu
    }
  }
  {
    auto def = SelectableButtonDef{
      .label = "Quit to Desktop",
      .size = size,
      .index = index++,
      .input = do_act,
      .sel_index = selected,
    };
    ImGui::NewLine();
    if (selectable_button(def))
      app.running = false;
  }

  ui_c.state.max = index;
  ImGui::End();
  ImGui::PopStyleVar();
};

} // namespace game2d