#include "ui_main_menu_playerjoin_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_helpers.hpp"

#include <imgui.h>
#include <steam/isteaminput.h>
#include <steam/steam_api_common.h>

namespace game2d {
using namespace std::literals;

void
update_ui_scene_main_menu_playerjoin_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, ui_e, ui_c);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);

  if (scene_c.s != Scene::menu)
    return;

  // Set action key set
  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_MenuControls);

  //
  // Clear the handles that have joined this frame
  // Reason: prevent clicking join, and then because
  // "get_button_down" would be true again,
  // the "play" button would immediately be clicked,
  // which I doubt is the users intention
  //
  ui_c.handles_joined_this_frame.clear();

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const float pos_x = viewport_pos.x + (ri.viewport_size_render_at.x * (9 / 12.0f));
  const float pos_y = viewport_pos.y + viewport_size_half.y;
  const auto pos = ImVec2(pos_x, pos_y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Players", NULL, flags);

  // There's a bug where the action handles aren't non-zero until a config is done loading. Soon config
  // information will be available immediately. Until then try to init as long as the handles are invalid.
  const auto& digital_action_handles = steam_c.digital_action_handles;
  if (digital_action_handles[(int)DA::Game_Up] == 0) {
    init_steam_input_actions(r);
    ImGui::Text("SteamInput not detected, or no controllers plugged in!");
    ImGui::End();
    return;
  }

  ImGui::Text("Connected Controllers: %i", steam_c.n_active);
  ImGui::Text("Assigned Controllers: %i", (int)non_zero_handles(ui_c.handles).size());
  ImGui::Separator();

  static std::unordered_map<InputHandle_t, std::string> join_key_map;

  for (int i = 0; i < steam_c.n_active; i++) {
    const InputHandle_t handle = steam_c.handles[i];
    if (handle == 0)
      continue;

    // ActionSet
    const auto& actionset_handles = steam_c.action_set_handles;
    const auto as = actionset_handles[(int)AS::ActionSet_MenuControls];

    // DigitalAction
    const auto& digital_action_handles = steam_c.digital_action_handles;
    const auto h = digital_action_handles[(int)DA::Menu_Select];

    EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS];
    const auto n_origins = SteamInput()->GetDigitalActionOrigins(handle, as, h, origins);
    if (n_origins > 0) {
      // use the first origin keyname
      EInputActionOrigin origin = origins[0];
      const char* keyname = SteamInput()->GetStringForActionOrigin(origin);
      join_key_map[handle] = keyname;
    }

    if (!join_key_map.contains(handle))
      join_key_map[handle] = "Loading...";

    auto b_join = controller_button_down(steam_c, handle, DA::Menu_Select);
    if (b_join) {
      assign_handle_to_ui(ui_c, handle);
      continue;
    }

    auto b_leave = controller_button_down(steam_c, handle, DA::Menu_Cancel);
    if (b_leave) {
      unassign_handle_from_ui(ui_c, handle);
      continue;
    }
  }

  // i.e. "waiting to assign"
  const auto free_controllers = connected_but_not_joined_controllers(steam_c, ui_c);
  int next_free_controller = 0;

  for (int i = 0; i < 4; i++) {
    ImGui::Text("%s", ("P"s + std::to_string(i)).c_str());

    const auto handle = ui_c.handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(ui_c, handle);

    if (joined && !connected) {
      ImGui::SameLine();
      ImGui::Text("Controller disconnected!");
      continue;
    }

    if (joined && connected) {
      ImGui::SameLine();
      // ImGui::Text("Connected. %zu", ui_c.handles[i]);
      ImGui::Text("Connected.");
      continue;
    }

    bool all_assigned = next_free_controller >= (int)free_controllers.size();
    if (free_controllers.size() == 0 || all_assigned) {
      ImGui::SameLine();
      ImGui::Text("N/A");
      continue; // no more free controllers
    }

    const auto unassigned_handle = free_controllers[next_free_controller++];
    const auto unassigned_handle_joinkey = join_key_map[unassigned_handle];
    const auto str = std::format("Press '{}' to join.", unassigned_handle_joinkey);

    ImGui::SameLine();
    ImGui::Text("%s", str.c_str());
  }

  ImGui::End();
}

} // namespace game2d