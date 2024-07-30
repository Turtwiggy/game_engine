#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/animation/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_entered_new_room/components.hpp"
#include "modules/ui_worldspace_sprite/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

namespace game2d {
using namespace std::literals;

void
create_imgui_quip(entt::registry& r, const entt::entity& e, const entt::entity& actor, const std::string& quip)
{
  auto& ui = r.emplace<WorldspaceTextComponent>(e);
  ui.text = quip;
  ui.offset.y = (-get_size(r, actor).y) / 2.0f;
  ui.split_text_into_lines = true;
  ui.line_length = 20;
  WiggleUpAndDown wig;
  wig.base_position = get_position(r, actor);
  wig.amplitude = 1.0;
  r.emplace<WiggleUpAndDown>(e, wig);
  r.emplace<EntityTimedLifecycle>(e, static_cast<int>(2.5f * 1000));
};

void
update_quips_system(entt::registry& r)
{
  if (get_first<SINGLE_QuipsComponent>(r) == entt::null)
    return;
  auto& quips = get_first_component<SINGLE_QuipsComponent>(r);

  static engine::RandomState rnd;

  //
  // Process ENTER_ROOM quip type
  // Damage request for QUIPs are in the resolve_collisions()
  //
  const auto map_e = get_first<MapComponent>(r);
  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (map_e != entt::null && dungeon_e != entt::null) {
    const auto& map = get_first_component<MapComponent>(r);
    const auto& dungeon = get_first_component<DungeonGenerationResults>(r);
    {
      const auto& reqs = r.view<PlayerEnteredNewRoom>();
      for (const auto& [req_e, req_c] : reqs.each()) {
        const auto& req_room = req_c.room;

        std::vector<entt::entity> enemies_in_newly_entered_room;

        // check the number of enemies in the room.
        const auto& enemies_view = r.view<EnemyComponent>();
        for (const auto& [enemy_e, enemy_c] : enemies_view.each()) {
          const auto enemy_pos = get_position(r, enemy_e);
          const auto enemy_gridpos = engine::grid::worldspace_to_grid_space(enemy_pos, map.tilesize);
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
  }

  //
  // Quip system
  //

  const auto& quip_req = r.view<RequestQuip>();
  for (const auto& [req_e, quip_c] : quip_req.each()) {
    const auto& e_to_quip = quip_c.quipp_e;

    // force reuse
    if (quips.quips_unused.size() == 0)
      quips.quips_unused = std::vector<std::string>(quips.quips.begin(), quips.quips.end());

    // force reuse
    if (quips.quips_hit_unused.size() == 0)
      quips.quips_hit_unused = std::vector<std::string>(quips.quips_hit.begin(), quips.quips_hit.end());

    // force reuse
    if (quips.quips_encounter_unused.size() == 0)
      quips.quips_encounter_unused = std::vector<std::string>(quips.quips_encounter.begin(), quips.quips_encounter.end());

    if (quip_c.type == QuipType::BEGIN_ENCOUNTER) {
      // pull the quip from the unused pile
      const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, int(quips.quips_encounter.size())));
      const auto quip = quips.quips_encounter[rnd_idx];
      quips.quips_encounter.erase(quips.quips_encounter.begin() + rnd_idx);

      // forward the quip text on to the worldspace text system
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      r.get<TagComponent>(e).tag = "empty_with_transform:Quip:BEGIN_ENCOUNTER";
      set_position(r, e, get_position(r, e_to_quip));
      set_size(r, e, { 0, 0 });

      create_imgui_quip(r, e, e_to_quip, quip);
    }

    if (quip_c.type == QuipType::ENTER_ROOM) {
      // pull the quip from the unused pile
      const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, int(quips.quips_unused.size())));
      const auto quip = quips.quips_unused[rnd_idx];
      quips.quips_unused.erase(quips.quips_unused.begin() + rnd_idx);

      // forward the quip text on to the worldspace text system
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      r.get<TagComponent>(e).tag = "empty_with_transform:Quip:ENTER_ROOM";
      set_position(r, e, get_position(r, e_to_quip));
      set_size(r, e, { 0, 0 });

      bool custom_sprite_text = false;
      if (custom_sprite_text) {
        auto& ui = r.emplace<WorldspaceSpriteRequestComponent>(e);
        ui.text = quip;
        ui.split_text_into_chunks = true;
        ui.chunk_length = 16;
        set_size(r, e, get_size(r, e_to_quip)); // size of enemy
      } else
        create_imgui_quip(r, e, e_to_quip, quip);
    }

    if (quip_c.type == QuipType::TOOK_DAMAGE) {
      // pull the quip from the unused pile
      const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, int(quips.quips_hit_unused.size())));
      const auto quip = quips.quips_hit_unused[rnd_idx];
      quips.quips_hit_unused.erase(quips.quips_hit_unused.begin() + rnd_idx);

      // forward the quip text on to the worldspace text system
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      r.get<TagComponent>(e).tag = "empty_with_transform:Quip:TOOK_DAMAGE";
      set_position(r, e, get_position(r, e_to_quip));
      set_size(r, e, { 0, 0 });

      bool custom_sprite_text = false;
      if (custom_sprite_text) {
        auto& ui = r.emplace<WorldspaceSpriteRequestComponent>(e);
        ui.text = quip;
        ui.split_text_into_chunks = true;
        ui.chunk_length = 16;
        set_size(r, e, get_size(r, e_to_quip)); // size of enemy
      } else
        create_imgui_quip(r, e, e_to_quip, quip);
    }
  }

  // destroy quip reqs...
  r.destroy(quip_req.begin(), quip_req.end());
};

} // namespace game2d