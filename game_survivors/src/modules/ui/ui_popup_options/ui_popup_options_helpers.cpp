#include "pch.hpp"

#include "ui_popup_options_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"

namespace game2d {

std::shared_ptr<IOption>
get_option(entt::registry& r, const GAME_OPTIONS o)
{
  const auto& options_c = get_first_component<SINGLE_GameOptions>(r);
  const auto& opts = options_c.options;

  const auto find_by_option = [o](std::shared_ptr<IOption> option) { return option->option == o; };
  const auto it = std::find_if(opts.begin(), opts.end(), find_by_option);
  if (it == opts.end())
    return nullptr;

  return (*it);
};

} // namespace game2d