#include "system.hpp"

#include "components.hpp"

#include "actors/helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_fov/components.hpp"


#include <fmt/core.h>

namespace game2d {

glm::vec2
get_rnd_worldpos_pos_in_room(const MapComponent& map, const Room& room)
{
  static engine::RandomState rnd;
  const int idx = engine::rand_det_s(rnd.rng, 0, (int)room.tiles_idx.size());
  const int room_idx = room.tiles_idx[idx];
  const auto wp = engine::grid::index_to_world_position_center(room_idx, map.xmax, map.ymax, map.tilesize);
  return wp;
};

glm::vec2
get_free_worldspace_pos_around_player(entt::registry& r, const MapComponent& map, const entt::entity e)
{
  const auto player_wp = get_position(r, get_first<PlayerComponent>(r));
  const auto player_gp = engine::grid::worldspace_to_grid_space(player_wp, map.tilesize);
  const auto neighbour_gps = engine::grid::get_neighbour_gridpos(player_gp, map.xmax, map.ymax);

  for (const auto& [dir, n_gp] : neighbour_gps) {
    bool in_bounds = engine::grid::grid_position_in_bounds(n_gp, map.xmax, map.ymax);
    if (!in_bounds)
      continue;
    const int n_idx = engine::grid::grid_position_to_index(n_gp, map.xmax);
    const auto& es = map.map[n_idx];

    // TODO: investigate this. stuff now contains loot on the floor,
    // which means that this cell WOULD technically be "free"
    if (es.size() != 0)
      continue;

    if (edge_between_gps(r, player_gp, n_gp) != entt::null)
      return engine::grid::grid_space_to_world_space_center(n_gp, map.tilesize);
  }

  // If no free gridpos, return your current pos
  return get_position(r, e);
};

glm::vec2
ai_decide_move_destination(entt::registry& r, const entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);

  if (get_first<PlayerComponent>(r) == entt::null)
    return get_position(r, e); // current position

  // Different AI systems to go here...

  /*
  Future interesting topics
  https://old.reddit.com/r/roguelikedev/comments/3b4wx2/faq_friday_15_ai/

  Specific patterns e.g.
  - Maintaining a certain distance / range. Min dst, max dst.
  - Pack tactics. Unit stays a minimum of 2 tiles from the player unless
    there are 4 units with a radius of 3 from the player.
  - Hit and run pack. Enemy stay ranged. Once they have minumum pack numbers,
    they charge in. Once they make an attack they switch back to ranged.
    Combine with healing enemies.

  - Go to Melee attack the player
  - Look for a good square to move to
  - Recency bias to encourage fun movement. e.g. avoid the previous 4 squares you've stepped on.
  - Have a goal. e.g. seeing player, noise, item

  - Temperment.
    e.g. always hostile,
    retreat after-attacked,
    hostile-after-attacked
  */

  // stupid simple ai for the moment.
  // Wander around, unless you've been spotted.
  // Then charge next to the player.

  const auto gp = get_grid_position(r, e);

  // You've not been spotted. Wander around your room.
  // Note: all entities are spawned in rooms.

  if (r.try_get<SeenComponent>(e) == nullptr) {
    const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

    const auto rooms = inside_room(r, gp);
    const bool in_room = rooms.size() > 0;
    if (in_room) {
      const auto& room_c = r.get<Room>(rooms[0]);

      // should be any valid tiles, but for the moment, just choose a random one
      return get_rnd_worldpos_pos_in_room(map, room_c);

    } else
    // If an entity somehow got in to a tunnel
    // without being seen (plausable later, but not currently)
    // then this would be a problem.
    {
      fmt::println("AI ERROR: entity in space??");
      return { 0.0f, 0.0f };
    }
  }

  // You've been spotted. Charge at the player.
  // Get the first grid position around the player.
  return get_free_worldspace_pos_around_player(r, map, e);
};

void
move_action(entt::registry& r, const entt::entity e)
{
  const glm::vec2 dst_wp = ai_decide_move_destination(r, e);

  move_action_common(r, e, dst_wp);
};

void
enemy_shoot_action(entt::registry& r, const entt::entity e)
{
  // aim your gun...
  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  r.emplace_or_replace<GunStaticTargetComponent>(e, get_position(r, player_e));

  if (r.try_get<SeenComponent>(e) == nullptr)
    return; // dont shoot, you're not seen

  // shoot if possible!
  r.emplace<WantsToShoot>(e);
}

void
update_turnbased_enemy_system(entt::registry& r)
{
  const auto state_e = get_first<SINGLE_CombatState>(r);
  if (state_e == entt::null)
    return;
  const auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team != AvailableTeams::enemy)
    return; // only process system if enemy turn

  // ImGui::Begin("Debug__TurnbasedEnemySystem");
  const bool one_at_a_time = true;
  bool all_enemies_fully_done = true;

  for (const auto& [e, e_c] : r.view<EnemyComponent>().each()) {
    // ImGui::PushID(static_cast<uint32_t>(e));

    auto& action_state = r.get_or_emplace<ActionState>(e);

    // Enemies have X actions able to take.
    // They could either e.g. move and shoot,
    // or move and move if it made more sense.
    // For the moment, just make enemies move and shoot.

    int& actions_available = action_state.actions_available;
    int& actions_completed = action_state.actions_completed;

    // Force first action to be move
    if (actions_available > 0 && actions_completed == 0) {
      actions_available--;
      actions_completed = 1;

      move_action(r, e);
    }

    // Force second action to be shoot.
    if (actions_available > 0 && actions_completed == 1) {
      const bool at_dest = at_destination(r, e);
      if (at_dest) {
        actions_available--;
        actions_completed = 2;

        enemy_shoot_action(r, e);
      }
    }

    const bool waiting_to_shoot = r.try_get<WantsToShoot>(e) != nullptr;
    const bool allowed_to_shoot = r.try_get<AbleToShoot>(e) != nullptr;

    // require all enemies to have done all actions
    all_enemies_fully_done &= actions_available == 0 && !waiting_to_shoot;

    if (waiting_to_shoot && !allowed_to_shoot) {
      // Possible causes: no gun. gun on cooldown. gun has no parent.
      // fmt::println("Potential AI error: waiting to shoot but hasn't shot yet. Maybe unable.");
    }

    // ImGui::PopID();
    if (one_at_a_time && (!all_enemies_fully_done))
      break;
  }
  // ImGui::End();

  // end the enemy turn
  if (all_enemies_fully_done) {
    create_empty<RequestToCompleteTurn>(r, RequestToCompleteTurn{ AvailableTeams::enemy });

    // Also, now all the enemies have moved, the fov needs updating
    create_empty<RequestUpdateFOV>(r);
  }
}

} // namespace game2d
