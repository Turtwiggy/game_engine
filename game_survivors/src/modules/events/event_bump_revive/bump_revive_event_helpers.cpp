#include "pch.hpp"

#include "bump_revive_event_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/actors/actor_islander/islander_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_anchor/anchor_helpers.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_revive/island_revive_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

void
handle_bump_event__revive(entt::registry& r, const BumpEvent& evt)
{
  if (!r.all_of<RevivableComponent>(evt.to))
    return;

  auto& tag_c = r.get<TagComponent>(evt.to);
  auto& revive_c = r.get<RevivableComponent>(evt.to);
  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
  auto& player_c = r.get<PlayerComponent>(evt.to);
  auto player_idx = player_c.idx;
  auto islander_e = evt.to;

  revive_c.hits_to_revive_cur++;

  if (revive_c.hits_to_revive_cur >= revive_c.hits_to_revive) {
    // do the revive.
    // which player needs reviving?
    SDL_Log("Revive player_idx %i", player_idx);
    dead_c.dead.push_back(evt.to); // kill the islander.

    const auto player_e = get_player_e_from_idx(r, player_idx);

    // reset health
    auto fixture_e = get_fixture_by_tag(r, player_e, "fixture_player");
    auto& hp_c = r.get<HealthComponent>(fixture_e);
    hp_c.hp = hp_c.max_hp;

    // remove positional tether
    remove_anchor(r, player_e);

    // you're alive and can move
    r.emplace<InputComponent>(player_e);
    r.remove<RevivableComponent>(player_e);

    // movementdirect gets removed when a player goes from boat => island.
    // If the boat died while a player was on the island, this component wont exist.
    // because the player never went through the return islander=>boat path.
    if (!r.all_of<MovementDirectComponent>(player_e))
      r.emplace<MovementDirectComponent>(player_e);

    // reset to default player speed
    const auto default_player_config = find_item(r, "actor_player");
    const auto default_speed = default_player_config.speed;
    r.get<ActorSpeedComponent>(player_e).current_speed = default_speed;
  }

  //
}

} // namespace game2d