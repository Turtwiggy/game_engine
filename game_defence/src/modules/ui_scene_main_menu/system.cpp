#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/controller.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/animation/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_level_editor/components.hpp"
#include "modules/ui_level_editor/helpers.hpp"
#include "modules/ui_rpg_character/components.hpp"
#include "modules/ux_hoverable_change_colour/components.hpp"
#include "physics/components.hpp"
#include "sprites/components.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <string>

namespace game2d {
using namespace std::literals;

void
update_ui_scene_main_menu(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& controllers = input.controllers;
  auto& ui = get_first_component<SINGLE_MainMenuUI>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  static int selected = 0;
  bool do_ui_action = false;

  // TODO: keyboard to update ui? mouse could just click.
  // Controller to update UI
  if (controllers.size() > 0) {
    auto* c = controllers[0];
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X))
      selected++;
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y))
      selected--;
    if (get_button_down(input, c, SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A))
      do_ui_action = true;
  }

  const int buttons = 2;
  selected = selected < 0 ? buttons - 1 : selected;
  selected %= buttons;

  const ImVec2 size = { 100, 75 };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, pivot);
  {
    ImGui::Selectable("Play", selected == 0, 0, size);
    if (ImGui::IsItemClicked()) {
      do_ui_action = true;
      selected = 0;
    }
    if (selected == 0 && do_ui_action) {

      // If you've clicked the play button, assume the game is playing live for real
      auto& editor = get_first_component<SINGLETON_LevelEditor>(r);
      editor.mode = LevelEditorMode::play;

      // configure params
      // ui.level = 1;

      // create request
      move_to_scene_start(r, Scene::game);

      // hack: load a level
      // load(r, "assets/maps/main.json");
    }

    ImGui::Selectable("Map Edit", selected == 1, 0, size);
    if (ImGui::IsItemClicked()) {
      do_ui_action = true;
      selected = 1;
    }
    if (selected == 1 && do_ui_action) {
      // If you've clicked the play button, assume the game is playing live for real
      auto& editor = get_first_component<SINGLETON_LevelEditor>(r);
      editor.mode = LevelEditorMode::edit;
      move_to_scene_start(r, Scene::game);
    }

    ImGui::Selectable("Quit", selected == 2, 0, size);
    if (ImGui::IsItemClicked()) {
      do_ui_action = true;
      selected = 2;
    }
    if (selected == 2 && do_ui_action)
      app.running = false;
  }
  ImGui::PopStyleVar();

  //   if (focused_element == 0)
  //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));
  //   if (ImGui::Button("Play".c_str(), button_size))
  //   if (focused_element == 0)
  //     ImGui::PopStyleColor();

  //   if (focused_element == 1)
  //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 0, 1));
  //   if (ImGui::Button("Quit", button_size))
  //     app.running = false;
  //   if (focused_element == 1)
  //     ImGui::PopStyleColor();

  // ImGui::NewLine();
  // for (int i = 0; i < 4; i++) {
  //   if (i > 0)
  //     ImGui::SameLine();
  //   ImGui::PushID(i);
  //   ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
  //   ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
  //   ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
  //   std::string tag = "Lv"s + std::to_string(i);
  //   if (ImGui::Button(tag.c_str())) {
  //     // DISABLED
  //     // ui.level = i;
  //     // editor.mode = LevelEditorMode::play;
  //     move_to_scene_start(r, Scene::game);
  //     // configure spawner for level
  //     // auto& grid = get_first_component<GridComponent>(r);
  //     // entt::entity spawner_e = grid.grid[0][0];
  //     // auto& spawner = r.get<SpawnerComponent>(spawner_e);
  //     // spawner.enemies_to_spawn = (i + 1) * 25;
  //     // load(r, "assets/maps/main.json");
  //     ui.instantiated_players.clear();
  //   }
  //   ImGui::PopStyleColor(3);
  //   ImGui::PopID();
  // }
  ImGui::End();

  // clang-format off
  // ImGui::Text("Aligned");
  // ImGui::SameLine(150); ImGui::Text("x=150");
  // ImGui::SameLine(300); ImGui::Text("x=300");
  // ImGui::Text("AlignedPotatoPotato");
  // ImGui::SameLine(150); ImGui::SmallButton("x=150");
  // ImGui::SameLine(300); ImGui::SmallButton("x=300");
  // clang-format on

  // draw X inactive-soliders wiggling
  const auto& monsters = r.view<CharacterStats, InActiveFight>();
  const int cur_size = monsters.size_hint();
  const int old_size = ui.instantiated_players.size();

  // Create Person
  const int to_create = cur_size - old_size;
  for (int i = 0; i < to_create; i++) {

    // Create Wiggly Person
    const int pixel_scale_up_size = 2;
    const auto default_size = glm::ivec2{ 16 * pixel_scale_up_size, 16 * pixel_scale_up_size };
    const auto e = create_gameplay(r, EntityType::actor_player);
    r.remove<PhysicsActorComponent>(e);
    r.emplace<ChangeColourOnHoverComponent>(e);

    // e.g. i=0,-200, i=1,0, i=2,200, i=3,400
    const auto scale = [](int i, int start_x, int increment) -> int { return start_x + increment * i; };

    // ideal pos's......  0 ---- 1 --(C)-- 2 ---- 3
    const int offset = old_size + (i); // old e.g. 3 existing + creating 0, 1, 2 as new position
    const float pos_x = scale(offset, -120, 80);

    auto& aabb = r.get<AABB>(e);
    aabb.center = { pos_x, 150 };

    auto& t = r.get<TransformComponent>(e);
    t.scale = { default_size.x, default_size.y, 0.0f };
    t.position = { aabb.center.x, aabb.center.y, 0.0f };

    WiggleUpAndDown wiggle;
    wiggle.base_position = aabb.center;
    wiggle.amplitude = 2.0f;
    r.emplace<WiggleUpAndDown>(e, wiggle);

    // Add stats to character
    for (int j = 0; const auto& [monster_e, stats, fight] : monsters.each()) {
      if (j == i) {
        r.emplace<CharacterStats>(e, stats);
        break;
      }
      j++;
    }

    ui.instantiated_players.push_back(e);
  }

  // Destroy Person
  for (auto i = old_size; i > cur_size; i--) {
    const auto idx = i - 1;
    // auto entity = ui.instantiated_players[idx];
    // r.destroy(entity); // fine because no aab
    // ui.instantiated_players.erase(ui.instantiated_players.begin() + idx);
  }

  // SQUAAAAAAAAAD
  //
  // ImGui::Begin("Squad");
  // ImGui::Text("Squad");
  // ImGui::NewLine();
  // for (int i = 0; i < ui.instantiated_players.size(); i++) {
  //   if (i > 0) {
  //     ImGui::SameLine();
  //     ImGui::Text(" - ");
  //     ImGui::SameLine();
  //   }
  //   // Get Location of text
  //   // Convert location to screenspace
  //   auto pos = ImGui::GetCursorPos();
  //   ImGui::Text("%f %f", pos.x, pos.y);
  //   auto pos2 = ImGui::GetCursorScreenPos();
  //   ImGui::Text("%f %f", pos2.x, pos2.y);
  //   if (const auto* stats = r.try_get<CharacterStats>(ui.instantiated_players[i]))
  //     ImGui::Text("%s", stats->name.c_str());
  // }
  // ImGui::End();

  //
  // In the top right, show a sound icon
  //
  ImGuiWindowFlags icon_flags = 0;
  icon_flags |= ImGuiWindowFlags_NoDocking;
  icon_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  icon_flags |= ImGuiWindowFlags_NoTitleBar;
  icon_flags |= ImGuiWindowFlags_NoCollapse;
  icon_flags |= ImGuiWindowFlags_NoResize;
  icon_flags |= ImGuiWindowFlags_NoMove;
  icon_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  // icon_flags |= ImGuiWindowFlags_NoNavFocus;

  // window settings
  //
  const int size_x = 1000;
  const int size_y = 500;
  const float cols_x = 20;
  const float cols_y = 10;
  const float pixels_x = size_x / cols_x;
  const float pixels_y = size_y / cols_y;
  const glm::ivec2 unmute_icon_offset{ 5, 0 };
  const glm::ivec2 mute_icon_offset{ 5, 1 };
  const ImVec2 icon_size = { 50, 50 };
  const float distance_from_right_side_of_screen = 75;
  static glm::ivec2 offset = unmute_icon_offset;
  static int toggle = 1;
  static bool toggle_changed = true;
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(
    ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - distance_from_right_side_of_screen, viewport->WorkPos.y));
  ImGui::Begin("Mute Sound Icon", nullptr, flags);

  // convert desired icon to uv coordinates
  // clang-format off
  ImVec2 tl = ImVec2(((offset.x * pixels_x + 0.0f    ) / size_x), ((offset.y * pixels_y + 0.0f    ) / size_y));
  ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / size_x), ((offset.y * pixels_y + pixels_y) / size_y));
  // clang-format on

  const auto tex_id = search_for_texture_id_by_path(ri, "kennynl_gameicons")->id;

  // draw an audio icon
  // ImGui::Image((ImTextureID)tex_id, { icon_size.x, icon_size.y }, tl, br);
  ImTextureID id = (ImTextureID)tex_id;
  int frame_padding = -1;
  if (ImGui::ImageButton(id, icon_size, tl, br, frame_padding, ImColor(0, 0, 0, 255))) {
    toggle = toggle == 1 ? 0 : 1;
    toggle_changed = true;
  }

  if (toggle == 0 && toggle_changed) {
    offset = unmute_icon_offset;
    auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
    audio.all_mute = false;

    // we're on the main menu... play the main menu audio
    r.emplace<AudioRequestPlayEvent>(r.create(), "MENU_01");
  }
  if (toggle == 1 && toggle_changed) {
    offset = mute_icon_offset;
    auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
    audio.all_mute = true;
    stop_all_audio(r);
  }
  toggle_changed = false;

  ImGui::End();
};

} // namespace game2d