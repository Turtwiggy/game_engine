#include "ui_scene_main_menu_system.hpp"

#include "ui_scene_main_menu_components.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_helpers.hpp"

#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <SDL2/SDL_log.h>
#include <string>

namespace game2d {
using namespace std::literals;

void
update_ui_scene_main_menu(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_MainMenuUI, r, ui_e, ui)
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_gs_e, steam_gs_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // left third centered
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto pos =
    ImVec2(viewport_pos.x + (ri.viewport_size_render_at.x * (3 / 12.0f)), viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  static int selected = 0;
  bool do_ui_action = false;

  //
  // Update menu via controller
  // Note: not assigned yet to SteamControllerComponent
  //

  const auto nz_handles = non_zero_handles(steam_gs_c.handles);

  for (int i = 0; i < (int)nz_handles.size(); i++) {
    const auto handle = nz_handles[i];

    if (handle_joined_this_frame(steam_gs_c, handle))
      continue; // prevent immediately doing do_ui_action

    if (controller_button_down(steam_c, handle, DA::Menu_Up))
      selected--;
    if (controller_button_down(steam_c, handle, DA::Menu_Down))
      selected++;
    if (controller_button_down(steam_c, handle, DA::Menu_Select))
      do_ui_action = true;
  }

  //
  // Update menu via keyboard (debug, mostly)
  //

  if (get_key_down(input, SDL_SCANCODE_KP_MINUS))
    selected--;
  if (get_key_down(input, SDL_SCANCODE_KP_PLUS))
    selected++;
  if (get_key_down(input, SDL_SCANCODE_KP_ENTER))
    do_ui_action = true;

  const auto& colors = ImGui::GetStyle().Colors;

  const auto b = ImGuiCol_Button;
  const auto bh = ImGuiCol_ButtonHovered;
  const auto ba = ImGuiCol_ButtonActive;

  const auto b_col = colors[b];
  const auto bh_col = colors[bh];
  const auto ba_col = colors[ba];

  const ImU32 button_col = IM_COL32(b_col.x * 255, b_col.y * 255, b_col.z * 255, b_col.w * 255);
  const ImU32 button_hovered_col = IM_COL32(bh_col.x * 255, bh_col.y * 255, bh_col.z * 255, bh_col.w * 255);
  const ImU32 button_clicked_col = IM_COL32(ba_col.x * 255, ba_col.y * 255, ba_col.z * 255, ba_col.w * 255);
  const ImU32 button_outline_col = button_hovered_col;

  const ImVec2 size = { 120.0f, 40.0f };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, pivot);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0)); // button hovered
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));  // button clicked

  const auto selectable_button = [&](const std::string& label, int& selected, const int index) {
    //
    // https://github.com/ocornut/imgui/issues/4719
    //
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);

    draw_list->ChannelsSetCurrent(1);
    if (ImGui::Selectable(label.c_str(), false, 0, size)) {
      selected = index;
      do_ui_action = true;
    }

    const bool is_hovered = ImGui::IsItemHovered();
    if (is_hovered)
      selected = index;
    bool is_selected = selected == index;

    draw_list->ChannelsSetCurrent(0);
    ImVec2 p_min = ImGui::GetItemRectMin();
    ImVec2 p_max = ImGui::GetItemRectMax();

    const bool is_clicked = ImGui::IsItemClicked(); // todo add controller
    const float rounding = 6.0;
    const float thickness = 2.0;
    const ImDrawFlags corners = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;

    if (is_clicked)
      ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_clicked_col, rounding);
    else if (is_hovered)
      ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_hovered_col, rounding);
    else
      ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, button_col, rounding);

    // draw an outline
    if (is_selected)
      ImGui::GetWindowDrawList()->AddRect(p_min, p_max, button_outline_col, rounding, corners, thickness);

    // "commit changes"
    draw_list->ChannelsMerge();

    play_sound_if_hovered(r, ui.hovered_buttons, label);

    // Do the callback for the button
    if (is_selected && do_ui_action) {
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_01" });
      return true;
    }

    return false;
  };

  int index = 0;

  if (selectable_button("Play", selected, index++))
    move_to_scene_start(r, Scene::select);

#if defined(_DEBUG)
  ImGui::NewLine();
  if (selectable_button("SpriteStack", selected, index++))
    move_to_scene_start(r, Scene::spritestack);
#endif

  ImGui::NewLine();
  if (selectable_button("Exit", selected, index++))
    app.running = false;

  // clamp selected
  selected = selected < 0 ? index - 1 : selected;
  selected %= index;

  ImGui::PopStyleVar(5);
  ImGui::PopStyleColor(2);
  ImGui::End();

  // note: could be in a separate file
  ui_mute_sound_icon(r);
};

} // namespace game2d