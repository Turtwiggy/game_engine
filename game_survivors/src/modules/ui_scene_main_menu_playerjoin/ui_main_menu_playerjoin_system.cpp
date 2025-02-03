#include "ui_main_menu_playerjoin_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"

#include <imgui.h>
#include <steam/isteaminput.h>
#include <steam/steam_api_common.h>

#include <ranges>

namespace game2d {
using namespace std::literals;

std::vector<InputHandle_t>
non_zero_handles(const std::vector<InputHandle_t>& handles)
{
  auto non_zero = [](const InputHandle_t h) { return h != 0; };
  auto non_zero_handles = handles | std::views::filter(non_zero);
  return std::vector(non_zero_handles.begin(), non_zero_handles.end());
};

bool
handle_is_connected(const SINGLE_SteamControllers& steam_c, const InputHandle_t handle)
{
  if (handle == 0)
    return false;
  auto it = std::find(steam_c.handles.begin(), steam_c.handles.end(), handle);
  return it != steam_c.handles.end();
};

bool
handle_is_joined(const SINGLE_SteamControllerGameState& ui_c, const InputHandle_t handle)
{
  if (handle == 0)
    return false;
  auto it = std::find(ui_c.handles.begin(), ui_c.handles.end(), handle);
  return it != ui_c.handles.end();
};

void
assign_handle_to_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle)
{
  if (handle_is_joined(ui_c, handle))
    return;

  for (int i = 0; i < ui_c.players; i++) {
    if (ui_c.handles[i] != 0)
      continue;
    ui_c.handles[i] = handle;
    break;
  }
};

void
unassign_handle_from_ui(SINGLE_SteamControllerGameState& ui_c, InputHandle_t handle)
{
  if (!handle_is_joined(ui_c, handle))
    return;
  auto it = std::find(ui_c.handles.begin(), ui_c.handles.end(), handle);
  const auto idx = static_cast<int>(it - ui_c.handles.begin());
  ui_c.handles[idx] = 0;
};

std::vector<InputHandle_t>
connected_but_not_joined_controllers(const SINGLE_SteamControllers& steam_c, const SINGLE_SteamControllerGameState& ui_c)
{
  std::vector<InputHandle_t> connected_but_not_joined;
  for (int i = 0; i < steam_c.n_active; i++) {
    const InputHandle_t handle = steam_c.handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(ui_c, handle);
    if (connected && !joined)
      connected_but_not_joined.push_back(steam_c.handles[i]);
  }
  return connected_but_not_joined;
};

//
// update
//

void
update_ui_scene_main_menu_playerjoin_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, ui_e, ui_c);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);

  if (scene_c.s != Scene::menu)
    return;

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
    set_steam_controller_action_set(r, handle, AS::ActionSet_MenuControls);

    // ActionSet
    const auto& actionset_handles = steam_c.action_set_handles;
    const auto as = actionset_handles[(int)AS::ActionSet_MenuControls];

    // DigitalAction
    const auto& digital_action_handles = steam_c.digital_action_handles;
    const auto h = digital_action_handles[(int)DA::Menu_JoinSlot];

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

    auto b_join = controller_button_held(steam_c, handle, DA::Menu_JoinSlot);
    if (b_join) {
      assign_handle_to_ui(ui_c, handle);
      continue;
    }

    // HACK: dont let unjoin if in survive scene?
    // if (scene_c.s != Scene::survive) {
    auto b_leave = controller_button_held(steam_c, handle, DA::Menu_LeaveSlot);
    if (b_leave) {
      unassign_handle_from_ui(ui_c, handle);
      continue;
    }
    // }
  }

  // i.e. "waiting to assign"
  const auto free_controllers = connected_but_not_joined_controllers(steam_c, ui_c);
  auto next_free_controller = (int)free_controllers.size();

  for (int i = 0; i < 4; i++) {
    std::string txt = ("P" + std::to_string(i));
    ImGui::Text("%s", txt.c_str());

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

    next_free_controller--;
    if (next_free_controller < 0) {
      ImGui::SameLine();
      ImGui::Text("N/A");
      continue; // no more free controllers
    }

    auto unassigned_handle = free_controllers[next_free_controller];
    auto unassigned_handle_joinkey = join_key_map[unassigned_handle];
    std::string str = std::format("Press {} to join.", unassigned_handle_joinkey);

    ImGui::SameLine();
    ImGui::Text("%s", str.c_str());
  }

  ImGui::End();

  // HACK: assign ui to components
  {
    const auto& group = r.group<PlayerComponent>();

    group.sort([&group](const entt::entity lhs, const entt::entity rhs) {
      const auto& a = group.get<PlayerComponent>(lhs);
      const auto& b = group.get<PlayerComponent>(rhs);
      return a.idx < b.idx;
    });

    int i = 0;
    for (const auto& [e, player_c] : group.each()) {
      auto& steam_controller_c = r.get<SteamControllerComponent>(e);
      steam_controller_c.handle = ui_c.handles[i];
      i++;
    }

    // Reset the rest of the components?
    // for (; i < 4; i++) {
    // }
  }
}

} // namespace game2d