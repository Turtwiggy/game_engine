#include "system.hpp"

#include "components.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "helpers.hpp"
#include "modules/persistent/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/scene_helpers.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <SDL2/SDL_log.h>
#include <format>
#include <string>

namespace game2d {
using namespace std::literals;

void
push_button_complete_colours()
{
  // Active state: Green
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.8f, 0.8f));
};

void
push_button_incomplete_colours()
{
  // Inactive state: red
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
};

void
eliminiation_level_x(entt::registry& r, int i, const ImVec2& size)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "kennynl_gameicons")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));

  ImGui::PushID(i);

  // load completed from disk
  bool completed = get_level_complete(r, i);

  std::string com_str = std::format("{}", i);
  if (completed)
    push_button_complete_colours();
  else
    push_button_incomplete_colours();

  if (ImGui::Button(com_str.c_str(), size)) {
    move_to_scene_start(r, Scene::dungeon_designer);
    create_empty<MenuToNextSceneInfo>(r, MenuToNextSceneInfo{ i });
  }

  ImGui::PopStyleColor(3);

  // Create an icon to show if the level is done or not
  const float icon_size = 18.0f;
  ImVec2 tl{ 0.0f, 0.0f };
  ImVec2 br{ 1.0f, 1.0f };
  const auto complete_sprite = completed ? "ICON_TICK" : "ICON_CROSS";
  const auto result = convert_sprite_to_uv(r, complete_sprite);
  std::tie(tl, br) = result;
  ImGui::SameLine(ImGui::GetContentRegionAvail().x - (icon_size * 0.5f) - 2);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((size.y - icon_size) * 0.5f)); // center icon vertically
  ImGui::Image(im_id, { icon_size, icon_size }, tl, br);

  ImGui::PopID();
}

void
update_ui_scene_main_menu(engine::SINGLE_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  auto& ui = get_first_component<SINGLE_MainMenuUI>(r);
  // const auto& input = get_first_component<SINGLE_InputComponent>(r);
  // const auto& controllers = input.controllers;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // center
  // const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  // const auto& viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  // const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  // ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  // left third centered
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto pos =
    ImVec2(viewport_pos.x + (ri.viewport_size_render_at.x * (3 / 12.0f)), viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  // TODO: keyboard to update ui? mouse could just click.
  // // Controller to update UI
  // if (controllers.size() > 0) {
  //   auto* c = controllers[0];
  //   if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X))
  //     selected++;
  //   if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y))
  //     selected--;
  //   if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A))
  //     do_ui_action = true;
  // }

  static int selected = 0;
  bool do_ui_action = false;
  // selected = selected < 0 ? buttons - 1 : selected;
  // selected %= buttons;

  const ImVec2 size = { 60, 20.0f };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

  const auto selectable_button = [&](const std::string& label, int& selected, const int index) {
    // update the selected index if this button is clicked
    if (ImGui::Button(label.c_str(), size)) {
      do_ui_action = true;
      selected = index;
    }

    play_sound_if_hovered(r, ui.hovered_buttons, label);

    // Do the callback for the button
    if (selected == index && do_ui_action) {
      create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_SELECT_01" });
      return true;
    }

    return false;
  };

  {
    int index = 0;

    // std::filesystem probably wont work on web. figure this out.
    // if (std::filesystem::exists("save-overworld.json")) {
    //   if (selectable_button("Continue", selected, index++))
    //     move_to_scene_start(r, Scene::overworld, true);
    // }

    // if (selectable_button("Start", selected, index++))
    //   move_to_scene_additive(r, Scene::overworld);

    // if (selectable_button("SETTINGS", selected, index++)) {
    // }

#if defined(_DEBUG)
    if (selectable_button("(debug) dungeon", selected, index++)) {
      move_to_scene_start(r, Scene::dungeon_designer);
      // create_empty<RequestGenerateDungeonComponent>(r);
    }
#endif

    // const auto table_flags = ImGuiTableFlags_Borders;
    if (ImGui::BeginTable("table2", 3)) {

      // Example data
      const int column_0_rows = 3;
      const int column_1_rows = 9;
      const int max_rows = std::max(column_0_rows, column_1_rows);
      int column_0_row = 0;
      int column_1_row = 0;

      for (int row = 0; row < max_rows; ++row) {
        ImGui::TableNextRow();

        // col 0: tutorial levels
        ImGui::TableSetColumnIndex(0);
        if (column_0_row == 0)
          ImGui::SeparatorText("Tutorials");

        if (row < column_0_rows) {
          const int i = column_0_row;
          ImGui::PushID(i);

          if (i == 0)
            ImGui::Text("In Progress");

          else
            ImGui::TextUnformatted("");

          ImGui::PopID();
          column_0_row++;
        } else
          ImGui::TextUnformatted(""); // empty cell

        // separator
        ImGui::TableSetColumnIndex(1);

        ImGui::TableSetColumnIndex(2);
        if (column_1_row == 0)
          ImGui::SeparatorText("Elimination");

        if (row < column_1_rows) {
          const int i = column_1_row + 1;

          // impl
          eliminiation_level_x(r, i, size);

          column_1_row++;
        } else {
          ImGui::TextUnformatted(""); // empty cell
        }
      }

      ImGui::EndTable();
    }

    // Exit button
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.6f, 0.8f, 0.8f));
    ImGui::NewLine();
    if (selectable_button("Exit", selected, index++))
      app.running = false;
    ImGui::PopStyleColor(3);
  }

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::End();

  // show a mute sound icon
  ui_mute_sound_icon(r);
};

} // namespace game2d