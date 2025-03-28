#include "pch.hpp"

#include "modules/ui_scene_main_menu/ui_scene_main_menu_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_animations/wiggle/components.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_scene_main_menu/helpers.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "modules/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

namespace game2d {
using namespace std::literals;

static glm::vec2 dropshadow_offset = { 6, 16 };
static float dropshadow_col[4] = {
  30 / 255.0f,
  50 / 255.0f,
  60 / 255.0f,
  0.5f,
};

void
text_with_dropshadow(std::string text, const ImVec4& col)
{
  const auto pos = ImGui::GetCursorPos();
  const auto dropshadow_pos = ImVec2{ pos.x + dropshadow_offset.x, pos.y + dropshadow_offset.y };

  // dropshadow first so it's below the text
  ImGui::SetCursorPos(dropshadow_pos);
  ImGui::TextColored(ImVec4(dropshadow_col[0], dropshadow_col[1], dropshadow_col[2], dropshadow_col[3]), "%s", text.c_str());

  // then the text
  ImGui::SetCursorPos(pos);
  ImGui::TextColored(col, "%s", text.c_str());
};

const auto init_menu = [](entt::registry& r) {
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
    //

    const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
    const auto font_enum = font_scale == 1.0f ? FontSize::HEADER : FontSize::HEADER_SCALED;
    auto* font = ImGui::GetIO().Fonts->Fonts[2]; // idx: 2 should be the fingerpaint header font

    ImGui::PushFont(font);

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
  // imgui_draw_vec2("dropshadow_offset", dropshadow_offset);
  // ImGui::ColorEdit4("dropshadow_col", dropshadow_col);
#endif

  if (!ui_c.init) {
    init_menu(r);
    ui_c.init = true;
  }

  process_requests<RequestToShowMainMenu>(r, [&ui_c]() {
    ui_c.one_frame_buffer = true;
    ui_c.display = true;
  });

  if (ui_c.one_frame_buffer) {
    ui_c.one_frame_buffer = false;
    return;
  }

  if (!ui_c.display)
    return;

  ImGuiIO& io = ImGui::GetIO();

  // button idx
  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::MENU_BUTTONS : FontSize::MENU_BUTTONS_SCALED;
  auto* font = get_fingerpaint_font(r, font_enum);
  ImGui::PushFont(font);

  const ImVec2 button_size = { 177.0f * font_scale, 50.0f * font_scale };
  const ImVec2 space_between_buttons = { 0, 16 * font_scale };

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

  // Problem statement:
  // We want to register buttons, and each button has an action

  if (!ui_c.state.init) {

    auto play_action = [&r]() { move_to_scene_start(r, Scene::select); };
    auto test_action = [&r]() { move_to_scene_start(r, Scene::procedural_snake); };
    auto upgrade_action = [&r, &ui_c]() {
      ui_c.display = false;
      ui_c.one_frame_buffer = true;
      // ui_c.state.current_row_index = 0;
      create_empty<RequestToShowUpgradesMenu>(r);
    };
    auto options_action = [&r, &ui_c]() {
      ui_c.display = false;
      ui_c.one_frame_buffer = true;
      // ui_c.state.current_row_index = 0;
      create_empty<RequestToShowOptionsMenu>(r);
    };
    auto exit_action = [&app]() { app.running = false; };

    ui_c.state.rows.push_back(RowState{ .col_name = "Play", .action = play_action });
    ui_c.state.rows.push_back(RowState{ .col_name = "Shipyard", .action = upgrade_action });
#if defined(_DEBUG)
    // ui_c.state.rows.push_back(RowState{ .col_name = "(Test) Snake", .action = test_action });
#endif
    ui_c.state.rows.push_back(RowState{ .col_name = "Options", .action = options_action });
    ui_c.state.rows.push_back(RowState{ .col_name = "Exit", .action = exit_action });

    ui_c.state.init = true;
  }

#if defined(_DEBUG)
  static bool debug_menu = false;
  if (debug_menu) {
    ImGui::Text("Menu Rows: %zu", ui_c.state.rows.size());
    for (const auto& row : ui_c.state.rows)
      ImGui::Text("%s, idx: %i", row.col_name.c_str(), row.col_index);
  }
#endif

  ImGui::Begin("Main Menu", nullptr, flags);

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);
  process_input_for_ui_all_handles(r, ui_c.state);

  int& selected = ui_c.state.current_row_index;
  const bool do_act = std::find(ui_c.state.new_actions.begin(), ui_c.state.new_actions.end(), UIAction::SELECT) !=
                      ui_c.state.new_actions.end();

  for (int i = 0; i < (int)ui_c.state.rows.size(); i++) {
    if (i > 0)
      ImGui::Dummy(space_between_buttons);

    auto& row = ui_c.state.rows[i];

    int col_idx = 0;

    auto a_def = SelectableButtonDef{
      .label = row.col_name,
      .size = button_size,
      .input = do_act,
      .my_row_index = i,
      .my_col_index = 0, // one col
      .ui_row_index = ui_c.state.current_row_index,
      .ui_col_index = col_idx, // one col
      .ui_col_active = true,   // one col
    };

    if (selectable_button(r, a_def))
      row.action();

    //
  }
  ImGui::PopStyleVar(5);
  ImGui::End();
  ImGui::PopFont();

  // note: could be in a separate file
  ui_mute_sound_icon(r);
};

} // namespace game2d