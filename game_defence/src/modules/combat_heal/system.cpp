#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/system_turnbased/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"

#include <SDL_scancode.h>

namespace game2d {

// hack: heal on q input
// this should be like if a medikit or something is used
void
update_combat_heal_system(entt::registry& r)
{
  const auto& player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;

  // limit: must be player turn
  auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team != AvailableTeams::player)
    return;

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_Q))
    r.emplace<WantsToHeal>(player_e);

  for (const auto& [e, req, hp, actions] : r.view<WantsToHeal, HealthComponent, ActionState>().each()) {
    if (actions.actions_available <= 0)
      continue;
    actions.actions_available--;
    actions.actions_completed++;

    // do the heal....
    hp.hp += 25;
    hp.hp = glm::min(hp.hp, hp.max_hp);

    // done req...
    r.remove<WantsToHeal>(e);
  }
}

} // namespace game2d