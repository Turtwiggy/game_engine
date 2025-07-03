#include "pch.hpp"

#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_scene_main_menu(engine::SINGLE_Application& app, entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;
  GET_FIRST_OR_RETURN(SINGLE_MainMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2((float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  if (!ui_c.init)
    ui_c.do_init(r);

  ui_c.update<RequestToShowMainMenu>(r);
  if (!ui_c.open)
    return;

  ImGuiIO& io = ImGui::GetIO();

  // button idx
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_LARGE : FontSize::TEXT_LARGE_SCALED;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.2f));

  const ImVec2 button_size = { 200.0f * font_scale, 50.0f * font_scale };
  const ImVec2 space_between_buttons = { 0, 4 * font_scale };

  const int n_buttons_y = 6;
  const auto size = ImVec2{ font_scale * 400, n_buttons_y * button_size.y };
  ImGui::SetNextWindowSize(size);

  imgui_begin("MainMenu");
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();

  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const bool do_act = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();

  int i = 0;
  const std::shared_ptr<Cell> root = ui_c.state.cells[0];
  std::shared_ptr<Cell> base = root;

  while (base->d != nullptr) {

    if (i > 0)
      ImGui::Dummy(space_between_buttons);

    const auto draw_button = [&](std::shared_ptr<Cell>& cell, int my_col_index) {
      auto a_def = SelectableButtonDef{
        .display_str = cell->name,
        .imgui_hash = "##" + cell->name,
        .size = button_size,
        .input = do_act,
        .cell = cell,
        .active_cell = ui_c.state.active,
        .font = font,

        .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      };
      if (selectable_button(r, a_def))
        cell->action();
    };

    const auto pos_x = (ui_wh.x * 0.5f) - (button_size.x * 0.5f);
    ImGui::SetCursorPosX(pos_x);

    draw_button(base, 0);

    if (base->r != nullptr) {
      ImGui::SameLine();
      std::shared_ptr<Cell>& cell = base->r;

      const auto icon_button_size = ImVec2{ 40.0f * font_scale, 40.0f * font_scale };
      auto tl = ImGui::GetCursorPos();
      tl.x += space_between_buttons.y;
      tl.y += button_size.y - icon_button_size.y;
      ImGui::SetCursorPos(tl);

      auto a_def = SelectableButtonDef{
        .display_str = "",
        .imgui_hash = "##" + cell->name,
        .icon = "ICON_WAND",
        .icon_size = { 32, 32 },
        .size = icon_button_size,
        .input = do_act,
        .cell = cell,
        .active_cell = ui_c.state.active,
        .font = font,

        .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      };
      if (selectable_button(r, a_def))
        cell->action();
    }

    base = base->d;
    if (base == root)
      break;
    i++;
  }
  ImGui::PopStyleVar(5);
  ImGui::End();
  ImGui::PopFont();

  // note: could be in a separate file
  // ui_mute_sound_icon(r);
};

} // namespace game2d