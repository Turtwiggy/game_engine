#include "modules/ui_scene_main_menu/ui_scene_main_menu_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_animations/wiggle/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
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

static glm::vec2 dropshadow_offset = { 6, 16 };
static float dropshadow_col[4] = {
  17 / 255.0f,
  19 / 255.0f,
  20 / 255.0f,
  0.5f,
};

void
text_with_dropshadow(std::string text, const ImVec4& col)
{
  const auto pos = ImGui::GetCursorPos();
  const auto dropshadow_pos = ImVec2{ pos.x + dropshadow_offset.x, pos.y + dropshadow_offset.y };

  // dropshadow first so it's below the text
  ImGui::SetCursorPos(dropshadow_pos);
  ImGui::TextColored(ImVec4(dropshadow_col[0], dropshadow_col[1], dropshadow_col[2], dropshadow_col[3]), "Oh Buoy!");

  // then the text
  ImGui::SetCursorPos(pos);
  ImGui::TextColored(col, "%s", text.c_str());
};

auto init_menu = [](entt::registry& r) {
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  //
  // create a wiggly header
  //

  // pos_x is 0 because camera is already at center
  const auto pos = glm::vec2(0, -viewport_size_half.y + ri.viewport_size_render_at.y * (3 / 12.0f));

  const auto my_greenish = hex_to_srgb("#71BBB2");
  const auto im_greenish = convert_my_to_im_vec(my_greenish);

  WorldspaceTextComponent wst_c;

  wst_c.layout = [im_greenish](entt::registry& r) {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[4]);

    text_with_dropshadow("Oh Buoy!", im_greenish);

    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
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

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

#if defined(_DEBUG)
  // imgui_draw_vec2("dropshadow", dropshadow_offset);
  ImGui::ColorEdit4("dropshadow_col", dropshadow_col);
#endif

  if (ui_c.one_frame_buffer) {
    ui_c.one_frame_buffer = false;
    init_menu(r);
    return;
  }

  ImGuiIO& io = ImGui::GetIO();

  // button idx
  ImGui::PushFont(io.Fonts->Fonts[5]);

  const ImVec2 size = { 204.0f, 62.0f };
  const ImVec2 space_between_buttons = { 0, 20 };

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.2f));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  ImGui::Begin("Main Menu", nullptr, flags);

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

  ImGui::Dummy(space_between_buttons);
  auto b_def = SelectableButtonDef{
    .label = "Options",
    .size = size,
    .index = index++,
    .input = do_act,
    .sel_index = selected,
  };
  if (selectable_button(b_def)) {
    create_empty<RequestToShowOptionsMenu>(r);
  }

  ImGui::Dummy(space_between_buttons);
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
  ImGui::End();
  ImGui::PopFont();

  // note: could be in a separate file
  ui_mute_sound_icon(r);
};

} // namespace game2d