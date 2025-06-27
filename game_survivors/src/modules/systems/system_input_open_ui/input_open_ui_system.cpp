#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_player/actor_player_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/systems/system_input_open_ui/input_open_ui_system.hpp"
#include "modules/ui/ui_popup_pause/ui_popup_pause_components.hpp"

namespace game2d {

void
update_input_open_ui_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  for (const auto [e, input_c] : r.view<const InputComponent>().each()) {
    if (has(input_c.pause, ActionStateEnum::DOWN))
      create_empty<RequestToShowPauseMenu>(r);
  }
}

} // namespace game2d
