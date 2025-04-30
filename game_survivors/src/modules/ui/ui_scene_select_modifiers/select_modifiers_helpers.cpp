#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "select_modifiers_helpers.hpp"

namespace game2d {

std::shared_ptr<IModifierOption>
get_modifier_option(entt::registry& r, const MODIFIER_OPTIONS o)
{
  const auto& options_c = get_first_component<SINGLE_ModifiersData>(r);
  const auto& opts = options_c.options;

  const auto find_by_option = [o](std::shared_ptr<IModifierOption> option) { return option->option == o; };
  const auto it = std::find_if(opts.begin(), opts.end(), find_by_option);
  if (it == opts.end())
    return nullptr;

  return (*it);
};

std::string
modifier_option_enum_to_display_string(entt::registry& r, const MODIFIER_OPTIONS o)
{
  auto opt_it = get_modifier_option(r, o);
  if (opt_it != nullptr)
    return opt_it->display_str;
  return "not impl";
  throw std::runtime_error("GAME_OPTION not handled.");
  return "not impl";
};

} // namespace game2d