#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "ui_scene_header_components.hpp"


namespace game2d {
using namespace std::literals;

template<class T>
void
show_if_in_menu(entt::registry& r, WorldspaceTextComponent& wst_c, const std::string& text)
{
  const auto ui_e = get_first<T>(r);

  T* ui_c = nullptr;
  if (ui_e != entt::null)
    ui_c = &r.get<T>(ui_e);

  if (ui_c && ui_c->open)
    wst_c.text = text;
}

void
update_ui_scene_header_system(entt::registry& r)
{
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);
  const auto megaheader_e = get_first<MegaHeaderComponent>(r);
  if (megaheader_e == entt::null)
    return;
  auto& txt_c = r.get<WorldspaceTextComponent>(megaheader_e);

  if (scene_c.s == Scene::menu)
    txt_c.text = "Oh Buoy!";

  show_if_in_menu<SINGLE_PersistentUpgradesMenuUI>(r, txt_c, "Upgrades!"s);
  show_if_in_menu<SINGLE_OptionsMenuState>(r, txt_c, "Options!"s);
  show_if_in_menu<SINGLE_UISelectModifiersMenuState>(r, txt_c, "Modifiers!"s);
}

} // namespace game2d