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
#include "modules/ui_worldspace_sprite/components.hpp"
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

  static engine::RandomState rnd;

  //
  // Process ENTER_ROOM quip type
  //
  {
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

      // choose a random enemy to say the quip
      const int enemy_to_quip_idx = int(engine::rand_det_s(rnd.rng, 0, enemies_in_newly_entered_room.size()));
      const entt::entity enemy_to_quip = enemies_in_newly_entered_room[enemy_to_quip_idx];

      // roll a dice to say a quip.
      const bool should_quip = engine::rand_01(rnd.rng) < 0.3f;
      if (!should_quip)
        return;

      // request quip
      RequestQuip quip_req;
      quip_req.type = QuipType::ENTER_ROOM;
      quip_req.quipp_e = enemy_to_quip;
      create_empty<RequestQuip>(r, quip_req);
    }

    // done all requests...
    r.destroy(reqs.begin(), reqs.end());
  }

  const auto& quip_req = r.view<RequestQuip>();
  for (const auto& [req_e, quip_c] : quip_req.each()) {
    const auto& e_to_quip = quip_c.quipp_e;

    // force reuse
    if (quips.quips_unused.size() == 0)
      quips.quips_unused = std::vector<std::string>(quips.quips.begin(), quips.quips.end());

    // force reuse
    if (quips.quips_hit_unused.size() == 0)
      quips.quips_hit_unused = std::vector<std::string>(quips.quips_hit.begin(), quips.quips_hit.end());

    if (quip_c.type == QuipType::ENTER_ROOM) {
      // pull the quip from the unused pile
      const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, int(quips.quips_unused.size())));
      const auto quip = quips.quips_unused[rnd_idx];
      quips.quips_unused.erase(quips.quips_unused.begin() + rnd_idx);

      // forward the quip text on to the worldspace text system
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      r.get<TagComponent>(e).tag = "empty_with_transform:Quip:ENTER_ROOM";

      auto& ui = r.emplace<WorldspaceSpriteRequestComponent>(e);
      ui.text = quip;
      ui.split_text_into_chunks = true;
      ui.chunk_length = 16;
      set_position(r, e, get_position(r, e_to_quip));
      set_size(r, e, get_size(r, e_to_quip));
    }

    if (quip_c.type == QuipType::TOOK_DAMAGE) {
      // pull the quip from the unused pile
      const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, int(quips.quips_hit_unused.size())));
      const auto quip = quips.quips_hit_unused[rnd_idx];
      quips.quips_hit_unused.erase(quips.quips_hit_unused.begin() + rnd_idx);

      // forward the quip text on to the worldspace text system
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      r.get<TagComponent>(e).tag = "empty_with_transform:Quip:TOOK_DAMAGE";

      auto& ui = r.emplace<WorldspaceSpriteRequestComponent>(e);
      ui.text = quip;
      ui.split_text_into_chunks = true;
      ui.chunk_length = 16;
      set_position(r, e, get_position(r, e_to_quip));
      set_size(r, e, get_size(r, e_to_quip));
    }
  }

  // destroy quip reqs...
  r.destroy(quip_req.begin(), quip_req.end());
};

} // namespace game2d