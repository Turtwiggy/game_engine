#include "pch.hpp"

#include "island_nearest_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_islanddweller/islanddweller_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"

namespace game2d {

void
update_island_nearest_system(entt::registry& r, glm::vec2 mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& islands_c = SINGLE_Islands::instance;
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  const int tilesize = SINGLE_Islands::instance.tilesize;

  // for (const auto& [id, eid] : islands_c.id_to_island_eid)
  //   set_colour(r, eid, { 1.0f, 1.0f, 1.0f, 1.0f });

  // mouse select an island tile
  /*
  {
    const auto worldpos = mouse_pos;
    const auto worldpos_adj = worldpos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
    const auto gridpos = engine::grid::worldspace_to_gridspace(worldpos_adj, tilesize);
    draw_sprite(r,
                Sprite{
                  .sprite = "EMPTY",
                  .pos = worldpos,
                  .size = { 5, 5 },
                  .z_idx = ZLayer::FOREGROUND,
                  .col = { 0.0f, 0.0f, 1.0f, 1.0f },
                });

    const auto id = engine::encode_cantor_pairing_function(gridpos.x, gridpos.y);
    if (islands_c.id_to_island_eid.contains(id)) {
      const auto island_eid = islands_c.id_to_island_eid.at(id);
      set_colour(r, island_eid, { 0.0f, 1.0f, 0.0f, 1.0f });
    }
  }
  */

  // Player's neighbour gridpos selct tiles.
  {
    for (const auto& [e, player_c, t_c, player_input_c, movement_c] :
         r.view<const PlayerComponent, const TransformComponent, const InputComponent, const MovementDirectComponent>()
           .each()) {

      const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
      const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
      const auto gridpos = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
      const auto n_gpos = engine::grid::get_neighbour_gridpos_with_diagonals(gridpos);

      for (const auto [grid_dir, n_gp] : n_gpos) {
        const auto id = engine::encode_cantor_pairing_function(n_gp.x, n_gp.y);

        if (!islands_c.id_to_island_eid.contains(id))
          continue;

        // you're now neighbouring an island
        // set_colour(r, island_eid, { 0.0f, 1.0f, 0.0f, 1.0f });
        const auto island_e = islands_c.id_to_island_eid.at(id);
        auto& island_c = r.get<DebugContoursComponent>(island_e);

        uint32_t x = 0;
        uint32_t y = 0;
        engine::decode_cantor_pairing_function(id, x, y);
        assert(x == n_gp.x && y == n_gp.y);

        const auto at_xy = [&](const std::pair<glm::ivec2, entt::entity>& data) { return data.first == glm::ivec2{ x, y }; };
        const auto it = std::find_if(island_c.occupied_island_xy.begin(), island_c.occupied_island_xy.end(), at_xy);
        if (it != island_c.occupied_island_xy.end())
          continue; // tile is occupied.

        // draw a sprite indicating you can land...
        auto n_pos = engine::grid::gridspace_to_worldspace(n_gp, tilesize);
        n_pos += glm::vec2{ tilesize, tilesize };
        draw_sprite(r,
                    Sprite{
                      .sprite = "EMPTY",
                      .pos = n_pos,
                      .size = { 5, 5 },
                      .z_idx = ZLayer::FOREGROUND,
                      .col = { 1.0f, 0.0f, 1.0f, 0.3f },
                    });

        // no inputs.
        if (!has(player_input_c.button_s, ActionStateEnum::DOWN))
          continue;

        // Spawn the player at the open space.
        auto island_player_e = spawn(r, "actor_islanddweller_player");
        give_life(r, island_player_e, n_pos, { tilesize, tilesize });

        // Add inputs to the island dweller.
        r.emplace<MovementIslandComponent>(island_player_e,
                                           MovementIslandComponent{
                                             .island_e = island_e,
                                             .boat_e = e,
                                           });
        if (r.all_of<SteamControllerComponent>(e))
          r.emplace<SteamControllerComponent>(island_player_e, r.get<SteamControllerComponent>(e));
        if (r.all_of<KeyboardComponent>(e))
          r.emplace<KeyboardComponent>(island_player_e, r.get<KeyboardComponent>(e));

        // Remove inputs from the boat.
        r.remove<MovementDirectComponent>(e);
        r.emplace<DroppedAnchorComponent>(e);
        // r.remove<InputComponent>(e); // dont remove input component

        // set the tile as occupied.
        island_c.occupied_island_xy.push_back({ n_gp, island_player_e });

        break; // give movement to one thing
      }
    }
  }
}

} // namespace game2d