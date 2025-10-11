#include "pch.hpp"

#include "island_movement_system.hpp"

#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/unordered_set/glm_hash.hpp"
#include "engine/std/vector/helpers.hpp"
#include "island_movement_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_bump/bump_event_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"

namespace game2d {

void
update_island_movement_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto view = r.view<const MovementIslandComponent, const TransformComponent, const InputComponent>();
  // ImGui::Text("There are %i things with MovementIslandComponent", (int)view.size_hint());

  for (const auto& [e, movement_c, t_c, input_c] : view.each()) {
    // set_colour(r, e, { 0.0f, 1.0f, 0.0f, 1.0f });

    const auto is_player = r.all_of<const PlayerComponent>(e);
    const bool move_l = has(input_c.dpad_l, ActionStateEnum::DOWN);
    const bool move_r = has(input_c.dpad_r, ActionStateEnum::DOWN);
    const bool move_u = has(input_c.dpad_u, ActionStateEnum::DOWN);
    const bool move_d = has(input_c.dpad_d, ActionStateEnum::DOWN);

    const auto island_e = movement_c.island_e;
    auto& island_c = r.get<DebugContoursComponent>(island_e);
    const auto tilesize = SINGLE_Islands::instance.tilesize;

    const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
    const auto gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);

    const std::vector<std::pair<bool, glm::ivec2>> dirs{
      { move_u, { 0, -1 } },
      { move_d, { 0, 1 } },
      { move_r, { 1, 0 } },
      { move_l, { -1, 0 } },
    };

    for (const auto& [move, dir] : dirs) {
      const auto n_gp = gp + dir;

      if (is_player) {
        const auto n_pos = engine::grid::gridspace_to_worldspace(n_gp, tilesize);
        const auto n_pos_adj = n_pos + glm::vec2{ tilesize, tilesize };
        const std::unordered_map<glm::ivec2, std::string> spritemap{
          { { 0, -1 }, "ARROW_UP" },
          { { 0, 1 }, "ARROW_DOWN" },
          { { 1, 0 }, "ARROW_RIGHT" },
          { { -1, 0 }, "ARROW_LEFT" },
        };

        draw_sprite(r,
                    Sprite{
                      .sprite = spritemap.at(dir),
                      .pos = n_pos_adj,
                      .size = { 12, 12 },
                      .col = { 0.0f, 1.0f, 0.0f, 1.0f },
                    });
      }

      if (!move)
        continue; // no input for this direction

      if (occupied(r, island_c, n_gp)) {
        // SDL_Log("tile is occupied...");

        const auto n_e = e_at_xy(r, island_c, n_gp);

        // make the neighbour flash.
        r.emplace_or_replace<RequestHitScaleComponent>(n_e);

        // send an event that you would bump in to something.
        {
          const BumpEvent evt{
            .from = e,
            .to = n_e,
            .from_gp = gp,
            .to_gp = n_gp,
          };
          const auto& evts_c = SINGLE_Events::instance;
          evts_c.dispatcher->trigger(evt);
          evts_c.dispatcher->update();
        }

        continue; // neighbour is full
      }

      if (!has(island_c.all_island_xy, n_gp)) {
        SDL_Log("tile is off the island...");

        // return to your boat.
        if (is_player)
          r.emplace_or_replace<WantToReturnToBoat>(e);

        continue; // you'd move off the island!
      }

      // It's possible that some damage has killed you.
      // SDL_Log("island dweller wants to move: %i %i", dir.x, dir.y);

      const auto it = std::find_if(island_c.occupied_island_xy.begin(),
                                   island_c.occupied_island_xy.end(),
                                   [&](const auto& other) { return other.first == gp; });

      if (it == island_c.occupied_island_xy.end()) {
        SDL_Log("Thing that wants to move is not in the occupied_island vector");
        // exit(1); // crash: you're not on the occupied island
        continue;
      }

      // remove at your current position
      island_c.occupied_island_xy.erase(it);

      // add to updated position.
      island_c.occupied_island_xy.push_back({ n_gp, e });

      // update transform
      auto new_pos = engine::grid::gridspace_to_worldspace(n_gp, tilesize);
      new_pos += glm::vec2{ tilesize, tilesize };
      LerpToFixedTarget lerp_c;
      lerp_c.a = pos;
      lerp_c.b = new_pos;
      lerp_c.speed = 15.0f;
      r.emplace_or_replace<LerpToFixedTarget>(e, lerp_c);

      break; // only move in 1 dir
    }
  }
}

} // namespace game2d