#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "ui_scene_header_components.hpp"

namespace game2d {

void
update_ui_scene_header_system(entt::registry& r)
{
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);
  const auto megaheader_e = get_first<MegaHeaderComponent>(r);
  if (megaheader_e == entt::null)
    return;
  auto& header_txt_c = r.get<WorldspaceTextComponent>(megaheader_e);

  if (scene_c.s == Scene::menu)
    header_txt_c.text = "Oh Buoy!";

  const auto menu_upgrade_e = get_first<SINGLE_PersistentUpgradesMenuUI>(r);
  SINGLE_PersistentUpgradesMenuUI* menu_upgrade_c = nullptr;
  if (menu_upgrade_e != entt::null)
    menu_upgrade_c = &r.get<SINGLE_PersistentUpgradesMenuUI>(menu_upgrade_e);
  if (menu_upgrade_c && menu_upgrade_c->open)
    header_txt_c.text = "Upgrades!";
}

} // namespace game2d