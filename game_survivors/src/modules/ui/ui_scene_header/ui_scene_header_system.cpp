#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui/ui_scene_select/scene_select_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "ui_scene_header_components.hpp"

namespace game2d {
using namespace std::literals;

template<class T>
void
show_if_in_menu(entt::registry& r,
                MegaHeaderComponent& header_c,
                WorldspaceTextComponent& wst_c,
                const std::string& text,
                const std::optional<std::string> icon = std::nullopt)
{
  const auto ui_e = get_first<T>(r);

  T* ui_c = nullptr;
  if (ui_e != entt::null)
    ui_c = &r.get<T>(ui_e);

  if (ui_c && ui_c->open) {
    wst_c.text = text;
  };

  if (icon.has_value() && ui_c && ui_c->open)
    header_c.icon = icon.value();
}

void
update_ui_scene_header_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& scene_c = SINGLE_CurrentScene::instance;
  const auto megaheader_e = get_first<MegaHeaderComponent>(r);
  if (megaheader_e == entt::null)
    return;
  auto& h_c = r.get<MegaHeaderComponent>(megaheader_e);
  auto& txt_c = r.get<WorldspaceTextComponent>(megaheader_e);

  h_c.icon = std::nullopt;

  if (scene_c.s == Scene::menu)
    txt_c.text = "Oh Buoy!";

  show_if_in_menu<SINGLE_PersistentUpgradesMenuUI>(r, h_c, txt_c, "Shipyard"s, "ICON_SAILBOAT");
  show_if_in_menu<SINGLE_OptionsMenuState>(r, h_c, txt_c, "Options"s, "ICON_WHEEL");
  show_if_in_menu<SINGLE_UISelectModifiersMenuState>(r, h_c, txt_c, "Modifiers"s, "ICON_WAND");

  auto select_e = get_first<SINGLE_SelectSceneData>(r);
  if (select_e != entt::null) {
    txt_c.text = "Prepare";
    // h_c.icon = "ICON_WHEEL";
  }

  const auto& ri_c = SINGLE_RendererInfo::instance;
  // idx: 2&3 should be the fingerpaint header font
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  auto* font = ImGui::GetIO().Fonts->Fonts[font_scale == 1.0f ? 2 : 3];
  const auto text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, txt_c.text.c_str());
  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);
  const auto zoom = camera_c.zoom_nonlinear;

  // pos_x is 0 because camera is already at center
  const auto half_wh = ImVec2(ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f);
  auto pos = glm::vec2(0, -half_wh.y + ri_c.viewport_size_render_at.y * (2.5 / 12.0f));
  pos -= 0.5f * glm::vec2{ text_size.x, text_size.y };
  pos *= zoom; // take in to account zoom.

  set_position(r, megaheader_e, pos);

  auto& wiggle_c = r.get_or_emplace<WiggleUpAndDown>(megaheader_e);
  if (wiggle_c.base_position != pos)
    wiggle_c.base_position = pos;
}

} // namespace game2d