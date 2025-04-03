#include "pch.hpp"

#include "ui_blur_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_components.hpp"
#include "modules/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

namespace game2d {

void
update_ui_blur_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input_c);
  GET_FIRST_OR_RETURN(SINGLE_CurrentScene, r, scene_e, scene_c);
  GET_FIRST_OR_RETURN(SINGLE_OptionsMenuState, r, options_e, options_c);
  GET_FIRST_OR_RETURN(SINGLE_PauseMenuState, r, pause_e, pause_c);
  const auto menu_upgrade_e = get_first<SINGLE_UpgradesMenuUI>(r);
  const auto game_upgrade_e = get_first<SINGLE_LevelUpUI>(r);

  SINGLE_UpgradesMenuUI* menu_upgrade_c = nullptr;
  SINGLE_LevelUpUI* game_upgrade_c = nullptr;
  if (menu_upgrade_e != entt::null)
    menu_upgrade_c = &r.get<SINGLE_UpgradesMenuUI>(menu_upgrade_e);
  if (game_upgrade_e != entt::null)
    game_upgrade_c = &r.get<SINGLE_LevelUpUI>(game_upgrade_e);

  const auto viewport_pos = ImVec2((float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y);
  const auto viewport_size = ImVec2(ri_c.viewport_size_render_at.x, ri_c.viewport_size_render_at.y);

  // temp: adjust blur.
  static float blur_amount = 0.0f;
  const float fade_in_speed = 4.0f;

  // blur with main menu...
  // if (scene_c.s == Scene::menu)
  //   blur_amount += fade_in_speed * dt;

  // blur with options menu...
  if (options_c.open)
    blur_amount += fade_in_speed * dt;

  // blur with pause menu...
  else if (pause_c.open)
    blur_amount += fade_in_speed * dt;

  // blur with upgrade menu...
  else if (menu_upgrade_c && menu_upgrade_c->display)
    blur_amount += fade_in_speed * dt;

  // blur with game upgrade menu...
  else if (game_upgrade_c && game_upgrade_c->open)
    blur_amount += fade_in_speed * dt;

  else
    blur_amount -= dt;

  blur_amount = glm::clamp(blur_amount, 0.0f, 1.0f);

  // static auto my_bg_col = hex_to_srgb("#27445D");
  // static auto im_bg_col = convert_my_to_im_vec(my_bg_col);
  // static float im_col[4] = { im_bg_col.x, im_bg_col.y, im_bg_col.z, im_bg_col.w };
  // ImGui::ColorEdit4("blur_col", im_col);
  // IM_COL32(im_col[0] * 255, im_col[1] * 255, im_col[2] * 255, im_col[3] * 255)

  const auto game_fade_col = IM_COL32(0, 0, 0, blur_amount * 0.5 * 255);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, game_fade_col);

  ImGui::SetNextWindowSize(viewport_size, ImGuiCond_Always);
  ImGui::SetNextWindowPos(viewport_pos);

  ImGuiWindowFlags blur_flags = 0;
  blur_flags |= ImGuiWindowFlags_NoDecoration;
  blur_flags |= ImGuiWindowFlags_NoCollapse;
  blur_flags |= ImGuiWindowFlags_NoTitleBar;
  blur_flags |= ImGuiWindowFlags_NoInputs;
  blur_flags |= ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("Blur", nullptr, blur_flags);
  ImGui::End();

  ImGui::PopStyleColor();
}

} // namespace game2d