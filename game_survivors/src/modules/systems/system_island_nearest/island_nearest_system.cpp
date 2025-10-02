#include "pch.hpp"

#include "island_nearest_components.hpp"
#include "island_nearest_helpers.hpp"
#include "island_nearest_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "resources/data.hpp"

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
#if defined(_DEBUG)
  {
    // const auto worldpos = mouse_pos;
    // const auto worldpos_adj = worldpos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
    // const auto gridpos = engine::grid::worldspace_to_gridspace(worldpos_adj, tilesize);
    // draw_sprite(r,
    //             Sprite{
    //               .sprite = "EMPTY",
    //               .pos = worldpos,
    //               .size = { 5, 5 },
    //               .z_idx = ZLayer::FOREGROUND,
    //               .col = { 0.0f, 0.0f, 1.0f, 1.0f },
    //             });
    // const auto id = engine::encode_cantor_pairing_function(gridpos.x, gridpos.y);
    // ImGui::Text("gp: %i, %i, id: %zu", gridpos.x, gridpos.y, id);
    // bool island = islands_c.id_to_island_eid.contains(id);
    // ImGui::Text("island: %s", island ? "true" : "false");
    // if (islands_c.id_to_island_eid.contains(id)) {
    //   const auto island_eid = islands_c.id_to_island_eid.at(id);
    //   set_colour(r, island_eid, { 0.0f, 1.0f, 0.0f, 1.0f });
    // }
  }
#endif

  // Player's neighbour gridpos selct tiles.
  {
    for (const auto& [e, player_c, t_c, player_input_c, movement_c] :
         r.view<const PlayerComponent, const TransformComponent, const InputComponent, const MovementDirectComponent>()
           .each()) {

      const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
      const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
      const auto gridpos = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
      const auto n_gpos = engine::grid::get_neighbour_gridpos_with_diagonals(gridpos);
      auto& nearest_c = r.get_or_emplace<IslandNearestComponent>(e);
      nearest_c.landable_positions.clear();

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

        // tile is occupied.
        if (occupied(r, island_c, n_gp))
          continue;

        // draw a sprite indicating you can land...
        auto n_pos = engine::grid::gridspace_to_worldspace(n_gp, tilesize);
        n_pos += glm::vec2{ tilesize, tilesize };

        // not sure about using player_c.idx
        auto col = default_player_colours[player_c.idx];

        draw_sprite(r,
                    Sprite{
                      .sprite = "EMPTY",
                      .pos = n_pos,
                      .size = { 5, 5 },
                      .z_idx = ZLayer::FOREGROUND,
                      // .col = { 1.0f, 0.0f, 1.0f, 0.3f },
                      .col = col,
                    });

        // keep track of landable positions
        nearest_c.landable_positions.push_back(n_pos);

        // no inputs.
        if (!has(player_input_c.button_s, ActionStateEnum::DOWN))
          continue;

        const auto boat_e = e;
        land_player_on_island(r, island_c, n_gp, boat_e, island_e);
        nearest_c.landable_positions.clear();

        break; // give movement to one thing
      }
    }
  }
}

} // namespace game2d