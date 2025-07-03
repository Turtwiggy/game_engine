#include "pch.hpp"

#include "island_movement_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/vector/helpers.hpp"
#include "island_movement_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_helpers.hpp"

namespace std {

template<>
struct hash<glm::ivec2>
{
  std::size_t operator()(const glm::ivec2& k) const { return (std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1)); }
};

} // namespace std

namespace game2d {

void
update_island_movement_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto view =
    r.view<const MovementIslandComponent, const TransformComponent, const InputComponent, const TeamComponent>();
  // ImGui::Text("There are %i things with MovementIslandComponent", (int)view.size_hint());

  for (const auto& [e, movement_c, t_c, input_c, team_c] : view.each()) {
    // set_colour(r, e, { 0.0f, 1.0f, 0.0f, 1.0f });

    const auto is_player = r.all_of<const PlayerComponent>(e);
    const bool move_l = has(input_c.dpad_l, ActionStateEnum::DOWN);
    const bool move_r = has(input_c.dpad_r, ActionStateEnum::DOWN);
    const bool move_u = has(input_c.dpad_u, ActionStateEnum::DOWN);
    const bool move_d = has(input_c.dpad_d, ActionStateEnum::DOWN);

    const auto tilesize = SINGLE_Islands::instance.tilesize;
    const auto island_e = movement_c.island_e;
    auto& island_c = r.get<DebugContoursComponent>(island_e);

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
                      .size = { 6, 6 },
                      .col = { 0.0f, 1.0f, 0.0f, 1.0f },
                    });
      }

      if (!move)
        continue; // no input for this direction

      if (occupied(r, island_c, n_gp)) {
        SDL_Log("tile is occupied...");

        const auto n_e = e_at_xy(r, island_c, n_gp);
        if (const auto* hp_c = r.try_get<const HealthComponent>(n_e)) {

          // dont damage friendly-team things
          const auto& neighbour_team_c = r.get<const TeamComponent>(n_e);
          const bool same_team = neighbour_team_c.team == team_c.team;
          if (same_team) {
            SDL_Log("A player collided with a friendly entity");
            continue;
          }

          // note: this is a grid-based damage system with no fixtures.
          const DamageEvent evt{
            .from = entt::null,
            .to_parent = n_e,
            .to_fixture = n_e, // same as parent, as no fixture
            .amount = 1,
            .type = WEAPON_DAMAGE::KINETIC,
          };
          const auto& evts_c = SINGLE_Events::instance;
          evts_c.dispatcher->trigger(evt);
          evts_c.dispatcher->update();
        }

        continue; // neighbour is full
      }

      if (!has(island_c.all_island_xy, n_gp)) {
        SDL_Log("tile is off the island...");
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

      // update transform (should improve this)
      auto new_pos = engine::grid::gridspace_to_worldspace(n_gp, tilesize);
      new_pos += glm::vec2{ tilesize, tilesize };
      set_position(r, e, new_pos);

      break; // only move in 1 dir
    }
  }
}

} // namespace game2d