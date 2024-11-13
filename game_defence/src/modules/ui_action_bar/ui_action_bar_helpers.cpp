#include "ui_action_bar_helpers.hpp"

#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_action_bar/ui_action_bar_components.hpp"

namespace game2d {

bool
any_unit_is_moving(entt::registry& r)
{
  bool moving = false;

  for (const auto& [e, name_c] : r.view<NameComponent>().each()) {
    const auto has_req = r.try_get<RequestMove>(e) != nullptr;
    const auto has_lerp = r.try_get<LerpToFixedTarget>(e) != nullptr;
    const auto& path_c = r.try_get<GeneratedPathComponent>(e);
    moving |= (has_lerp || has_req || path_c);
  }

  return moving;
};

} // namespace game2d