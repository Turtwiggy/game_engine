#include "pch.hpp"

#include "ui_back_button_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
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
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);
  GET_FIRST_OR_RETURN(SINGLE_CurrentScene, r, scene_e, scene_c);
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);

  auto& ui_c = gesert_component<UI_BackButton>(r);

  if (!ui_c.init)
    ui_c.do_init(r);

  bool something_open = false;

  //
  // which ui to show the back button?
  //

  if (get_first<SINGLE_OptionsMenuState>(r) != entt::null) {
    const auto& ui_options_c = get_first_component<SINGLE_OptionsMenuState>(r);
    something_open |= ui_options_c.open;
  }

  if (get_first<SINGLE_PersistentUpgradesMenuUI>(r) != entt::null) {
    const auto& ui_modifiers_c = get_first_component<SINGLE_PersistentUpgradesMenuUI>(r);
    something_open |= ui_modifiers_c.open;
  }

  if (get_first<SINGLE_UISelectModifiersMenuState>(r) != entt::null) {
    const auto& ui_modifiers_c = get_first_component<SINGLE_UISelectModifiersMenuState>(r);
    something_open |= ui_modifiers_c.open;
  }

  //
  // which scene to show the back button?
  //

  if (scene_c.s == Scene::select_ships)
    something_open |= true;

  if (!something_open)
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

  const auto button_size = ImVec2{ 120, 30 };

  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto text_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto text_font_size = (float)text_font_enum;
  auto* text_font = get_inter_font(r, text_font_enum);

  ImGui::SetNextWindowPos({ (float)ri_c.viewport_size_render_at.x - 10, (float)ri_c.viewport_size_render_at.y - 10 },
                          ImGuiCond_Always,
                          { 1.0f, 1.0f });
  ImGui::SetNextWindowSize({ button_size.x + 10, button_size.y + 10 }, ImGuiCond_Appearing);

  ImGui::Begin("BACK", 0, flags);

  auto& cell = ui_c.state.cells[0];

  SelectableButtonDef def{
    .label = cell->name,
    .size = button_size,
    .cell = cell,
    .active_cell = ui_c.state.active,
    .update_selected_only_with_mouse = true,
    .font = text_font,

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

  if (get_key_down(input_c, SDL_SCANCODE_ESCAPE))
    move_to_scene_start(r, Scene::menu);
}

} // namespace game2d