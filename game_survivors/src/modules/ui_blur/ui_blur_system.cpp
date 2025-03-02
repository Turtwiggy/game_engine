#include "ui_blur_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "imgui.h"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_components.hpp"

#include <SDL_scancode.h>

namespace game2d {

void
update_ui_blur_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input_c);
  GET_FIRST_OR_RETURN(SINGLE_CurrentScene, r, scene_e, scene_c);
  GET_FIRST_OR_RETURN(SINGLE_OptionsMenuState, r, options_e, options_c);
  GET_FIRST_OR_RETURN(SINGLE_PauseMenuState, r, pause_e, pause_c);

  const auto viewport_pos = ImVec2((float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y);
  const auto viewport_size = ImVec2(ri_c.viewport_size_render_at.x, ri_c.viewport_size_render_at.y);

  // temp: adjust blur.
  static float blur_amount = 0.0f;

  // blur with main menu...
  if (scene_c.s == Scene::menu)
    blur_amount += dt;

  // blur with options menu...
  else if (options_c.open)
    blur_amount += dt;

  // blur with pause menu...
  else if (pause_c.open)
    blur_amount += dt;

  else
    blur_amount -= dt;

  blur_amount = glm::clamp(blur_amount, 0.0f, 0.5f);

  const auto my_bg_col = hex_to_srgb("#27445D");
  const auto im_bg_col = convert_my_to_im(my_bg_col);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, im_bg_col);

  ImGui::SetNextWindowSize(viewport_size, ImGuiCond_Always);
  ImGui::SetNextWindowPos(viewport_pos);
  ImGui::SetNextWindowBgAlpha(blur_amount);

  ImGuiWindowFlags blur_flags = 0;
  blur_flags |= ImGuiWindowFlags_NoDecoration;
  blur_flags |= ImGuiWindowFlags_NoCollapse;
  blur_flags |= ImGuiWindowFlags_NoTitleBar;
  blur_flags |= ImGuiWindowFlags_NoInputs;

  ImGui::Begin("Blur", nullptr, blur_flags);
  ImGui::Text("Blur Amount: %f", blur_amount);
  ImGui::End();

  ImGui::PopStyleColor();
}

} // namespace game2d