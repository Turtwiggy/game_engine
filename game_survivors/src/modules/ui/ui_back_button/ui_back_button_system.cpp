#include "pch.hpp"

#include "ui_back_button_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"
#include "ui_back_button_components.hpp"

namespace game2d {

void
update_ui_back_button_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& ri_c = SINGLE_RendererInfo::instance;
  ;
  const auto& scene_c = SINGLE_CurrentScene::instance;
  const auto& input_c = SINGLE_InputComponent::instance;

  auto& ui_c = gesert_component<UI_BackButton>(r);

  if (!ui_c.init)
    ui_c.do_init(r);

  bool something_open = false;
  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_sel = g_input_c.select;
  bool do_act = false;
  do_act |= std::find(b_sel.begin(), b_sel.end(), ActionStateEnum::DOWN) != b_sel.end();
  do_act |= get_key_down(input_c, SDL_SCANCODE_ESCAPE);

  //
  // which ui to show the back button?
  //

  auto options_e = get_first<SINGLE_OptionsMenuState>(r);
  auto upgrades_e = get_first<SINGLE_PersistentUpgradesMenuUI>(r);
  auto modifiers_e = get_first<SINGLE_UISelectModifiersMenuState>(r);

  if (options_e != entt::null) {
    const auto& ui_options_c = get_first_component<SINGLE_OptionsMenuState>(r);
    something_open |= ui_options_c.open;
  }

  if (upgrades_e != entt::null) {
    const auto& ui_modifiers_c = get_first_component<SINGLE_PersistentUpgradesMenuUI>(r);
    something_open |= ui_modifiers_c.open;
  }

  if (modifiers_e != entt::null) {
    const auto& ui_modifiers_c = get_first_component<SINGLE_UISelectModifiersMenuState>(r);
    something_open |= ui_modifiers_c.open;
  }

  //
  // which scene to show the back button?
  //

  if (scene_c.s == Scene::select_ships)
    something_open |= true;

  // only show the back button if something is open
  if (!something_open)
    return;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

  const auto button_size = ImVec2{ 200, 30 };

  const auto font_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_scale = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);

  ImGui::SetNextWindowPos({ (float)ri_c.viewport_size_render_at.x - 10, (float)ri_c.viewport_size_render_at.y - 10 },
                          ImGuiCond_Always,
                          { 1.0f, 1.0f });
  ImGui::SetNextWindowSize({ button_size.x + 10, button_size.y + 10 }, ImGuiCond_Appearing);

  imgui_begin("BACK");

  auto& cell = ui_c.state.cells[0];

  SelectableButtonDef def{
    .display_str = cell->name,
    .imgui_hash = "##" + cell->name,
    .size = button_size,
    .cell = cell,
    .active_cell = ui_c.state.active,
    .update_selected_only_with_mouse = true,
    .font = font,
    .font_size = font_scale,

    // hide the buttons
    .active_outline_col = { 1.0f, 1.0f, 1.0f, 0.4f },
    .inactive_outline_col = { 1.0f, 1.0f, 1.0f, 0.4f },
    .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
  };

  ImGui::SetCursorPos({ 5, 5 }); // padding
  if (selectable_button(r, def))
    move_to_scene_start(r, Scene::menu);

  ImGui::End();
  ImGui::PopStyleVar(4);

  const auto& cur_s = SINGLE_CurrentScene::instance;
  if (do_act) {

    if (cur_s.s == Scene::survive) {
      // close the open menu.
      if (options_e != entt::null)
        get_first_component<SINGLE_OptionsMenuState>(r).open = false;
    }

    else
      move_to_scene_start(r, Scene::menu);
  }
}

} // namespace game2d