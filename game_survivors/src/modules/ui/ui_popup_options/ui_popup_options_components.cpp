#include "pch.hpp"

#include "ui_popup_options_components.hpp"

#include "modules/core/options/options_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "ui_popup_options_helpers.hpp"

namespace game2d {

std::string
game_option_enum_to_display_string(entt::registry& r, const GAME_OPTIONS o)
{
  auto opt_it = get_option(r, o);
  if (opt_it != nullptr)
    return opt_it->display_str;
  return "not impl";
  throw std::runtime_error("GAME_OPTION not handled.");
  return "not impl";
};

int
game_option_enum_load_from_prefs(entt::registry& r, const GAME_OPTIONS o)
{
  auto opt_it = get_option(r, o);
  if (opt_it == nullptr)
    return 0;
  return opt_it->get_hindex(r);
};

void
SINGLE_OptionsMenuState::do_init(entt::registry& r)
{
  for (int i = 0; i < (int)GAME_OPTIONS::count; i++) {
    const auto enum_val = magic_enum::enum_cast<GAME_OPTIONS>(i).value();
    const auto enum_str = std::string(magic_enum::enum_name(enum_val));

    OptionsCell oc;
    oc.name = game_option_enum_to_display_string(r, enum_val);
    oc.value = game_option_enum_load_from_prefs(r, enum_val);
    state.cells.push_back(std::make_shared<OptionsCell>(oc));
  }

  //
  // nav
  //
  create_as_vertical_layout(state.cells);

  state.active = state.cells[0];
  init = true;
}

} // namespace game2d