#include "modules/ui_scene_main_menu/ui_scene_main_menu_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_animations/wiggle/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_components.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_scene_main_menu/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include <SDL2/SDL_log.h>
#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {
using namespace std::literals;

auto init_menu = [](entt::registry& r) {
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  //
  // create a wiggly header
  //

  // pos_x is 0 because camera is already at center
  const auto pos = glm::vec2(0, -viewport_size_half.y + ri.viewport_size_render_at.y * (3 / 12.0f));

  WorldspaceTextComponent wst_c;

  wst_c.layout = [](entt::registry& r) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[4]);

    auto my_greenish = hex_to_srgb("#71BBB2");
    auto im_greenish = convert_my_to_im(my_greenish);
    ImGui::TextColored(im_greenish, "Oh Buoy!");

    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
  wst_c.flags |= ImGuiWindowFlags_NoNav;
  wst_c.flags |= ImGuiWindowFlags_NoBackground;

  auto header_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(header_e);
  r.emplace<WiggleUpAndDown>(header_e,
                             WiggleUpAndDown{
                               .base_position = pos,
                             });
  set_position(r, header_e, pos);
};

void
update_ui_scene_main_menu(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_MainMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  if (ui_c.one_frame_buffer) {
    ui_c.one_frame_buffer = false;
    init_menu(r);
    return;
  }

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  ImGuiIO& io = ImGui::GetIO();

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // button idx
  ImGui::PushFont(io.Fonts->Fonts[5]);

  // centered
  const auto pos =
    ImVec2(viewport_pos.x + (ri.viewport_size_render_at.x * (6 / 12.0f)), viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Main Menu", nullptr, flags);

  const ImVec2 size = { 120.0f, 40.0f };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, pivot);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0)); // button hovered
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));  // button clicked

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);
  process_input_for_ui(r, ui_c.state);
  bool& do_act = ui_c.state.do_action;
  int& selected = ui_c.state.selected;

  int index = 0;

  auto a_def = SelectableButtonDef{
    .label = "Play",
    .size = size,
    .index = index++,
    .input = do_act,
    .sel_index = selected,
  };
  if (selectable_button(a_def))
    move_to_scene_start(r, Scene::select);

  ImGui::NewLine();
  auto c_def = SelectableButtonDef{
    .label = "Exit",
    .size = size,
    .index = index++,
    .input = do_act,
    .sel_index = selected,
  };
  if (selectable_button(c_def))
    app.running = false;

  ui_c.state.max = index;
  ImGui::PopStyleVar(5);
  ImGui::PopStyleColor(2);
  ImGui::End();
  ImGui::PopFont();

  // note: could be in a separate file
  ui_mute_sound_icon(r);
};

} // namespace game2d