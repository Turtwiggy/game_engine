#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_entered_new_room/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include "fmt/core.h"

namespace game2d {
using namespace std::literals;

void
update_quips_system(entt::registry& r)
{
  if (get_first<MapComponent>(r) == entt::null)
    return;
  if (get_first<DungeonGenerationResults>(r) == entt::null)
    return;
  if (get_first<SINGLE_QuipsComponent>(r) == entt::null)
    return;

  const auto& map = get_first_component<MapComponent>(r);
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);
  auto& quips = get_first_component<SINGLE_QuipsComponent>(r);

  const auto& reqs = r.view<PlayerEnteredNewRoom>();
  for (const auto& [req_e, req_c] : reqs.each()) {
    const auto& req_room = req_c.room;

    std::vector<entt::entity> enemies_in_newly_entered_room;

    // check the number of enemies in the room.
    const auto& enemies_view = r.view<EnemyComponent>();
    for (const auto& [enemy_e, enemy_c] : enemies_view.each()) {
      const auto enemy_pos = get_position(r, enemy_e);
      const auto enemy_gridpos = engine::grid::world_space_to_grid_space(enemy_pos, map.tilesize);
      const auto [in_room, room] = inside_room(map, dungeon.rooms, enemy_gridpos);
      if (!in_room)
        continue; // enemy not in a room (probably in a tunnel?)
      if (room->tl != req_room.tl)
        continue; // enemy not in the room we just entered
      enemies_in_newly_entered_room.push_back(enemy_e);
    }

    // must be enemies
    if (enemies_in_newly_entered_room.size() == 0)
      continue;

    static engine::RandomState rnd;

    // roll a dice to say a quip.
    // const float percentage_chance_to_quip = 30;
    // const bool should_quip = int(engine::rand_det_s(rnd.rng, 0, 100)) < percentage_chance_to_quip;
    // if (!should_quip)
    //   return;

    // choose a random enemy to say the quip
    const int enemy_to_quip_idx = int(engine::rand_det_s(rnd.rng, 0, enemies_in_newly_entered_room.size()));
    const entt::entity enemy_to_quip = enemies_in_newly_entered_room[enemy_to_quip_idx];
    // fmt::println("quipping enemy: {}", enemy_to_quip_idx);

    // force reuse
    if (quips.quips_unused.size() == 0)
      quips.quips_unused = std::vector<std::string>(quips.quips.begin(), quips.quips.end());

    // pull the quip from the unused pile
    const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, quips.quips_unused.size()));
    const std::string quip = quips.quips_unused[rnd_idx];
    quips.quips_unused.erase(quips.quips_unused.begin() + rnd_idx);

    // quip!
    const auto e = create_gameplay(r, EntityType::empty_with_transform);
    r.emplace<EntityTimedLifecycle>(e, 3 * 1000);
    auto& ui = r.emplace<WorldspaceTextComponent>(e);
    ui.text = quip;
    ui.offset.y = (-get_size(r, enemy_to_quip).y / 2.0f) - 20;
    set_position(r, e, get_position(r, enemy_to_quip));
    set_size(r, e, { 0, 0 });
  }

  // done all requests... and generated all quips
  r.destroy(reqs.begin(), reqs.end());
};

} // namespace game2d