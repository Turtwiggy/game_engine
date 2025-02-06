#include "helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"


namespace game2d {

void
drop_xp_on_death_callback(entt::registry& r, const entt::entity e)
{
  const auto size = glm::vec2{ 8, 8 };

  const auto item_e = spawn(r, "xp");
  give_life(r, item_e, get_position(r, e), size);
  r.emplace<TeamComponent>(item_e, AvailableTeams::neutral);
  r.emplace<XpComponent>(item_e);

  // xp doesnt do anything on it's death
  r.remove<OnDeathCallbacks>(item_e);
};

} // namespace game2d