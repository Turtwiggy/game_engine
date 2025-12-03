#include "pch.hpp"

#include "island_nearest_components.hpp"
#include "island_nearest_helpers.hpp"
#include "island_nearest_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/unordered_set/glm_hash.hpp"
#include "engine/std/vector/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/systems/system_island_return_to_boat_land_immunity/island_return_to_boat_land_immunity_components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_island_nearest_system(entt::registry& r, glm::vec2 mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& islands_c = SINGLE_Islands::instance;
  const auto& input_c = SINGLE_InputComponent::instance;
  const float tilesize = SINGLE_Islands::instance.tilesize;

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
    const auto view = r.view<const PlayerComponent,
                             const TransformComponent,
                             const InputComponent,
                             const MovementDirectComponent,
                             const PhysicsBodyComponent>();
    for (const auto& [e, player_c, t_c, input_c, m_c, body_c] : view.each()) {

      auto& nearest_c = r.get_or_emplace<IslandNearestComponent>(e);
      nearest_c.landable_positions.clear();

      // const auto& wh = t_c.scale;
      const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
      const auto fixture_e = body_c.fixtures[0]; // assume first fixture is player(!)
      const auto fixture_c = r.get<const PhysicsFixtureComponent>(fixture_e);
      const auto aabb = b2Shape_GetAABB(fixture_c.shapeId);
      const auto size = meters_to_pixels({ aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y });
      const auto tl = pos - (0.5f * size);
      const auto br = pos + (0.5f * size);

      // move x (and y) to the left-most, nearest grid position
      // e.g. for tilesize 25, if x is 52, move it to 50.
      const float x_min = ((std::floor(tl.x / tilesize)) * tilesize) - 1;
      const float y_min = ((std::floor(tl.y / tilesize)) * tilesize) - 1;

      // move x (and y) to the right-most, nearest grid position
      // e.g. for tilesize 25, if x is 52, move it to 75.
      const float x_max = (std::ceil(br.x / tilesize) * tilesize) + 1;
      const float y_max = (std::ceil(br.y / tilesize) * tilesize) + 1;

#if defined(_DEBUG)
/*
      {
        Sprite spr;
        spr.sprite = "EMPTY";
        spr.size = { 6, 6 };
        spr.pos = tl;
        spr.col = { 1.0f, 0.0, 0.0, 1.0 };
        draw_sprite(r, spr);
      }

      {
        Sprite spr;
        spr.sprite = "CAT";
        spr.size = { 16, 16 };
        spr.pos = { x_min, y_min };
        spr.col = { 0.0f, 1.0, 0.0, 1.0 };
        draw_sprite(r, spr);
      }

      {
        Sprite spr;
        spr.sprite = "EMPTY";
        spr.size = { 6, 6 };
        spr.pos = br;
        spr.col = { 0.0f, 0.0, 1.0, 1.0 };
        draw_sprite(r, spr);
      }

      {
        Sprite spr;
        spr.sprite = "CAT";
        spr.size = { 16, 16 };
        spr.pos = { x_max, y_max };
        spr.col = { 0.0f, 1.0, 1.0, 1.0 };
        draw_sprite(r, spr);
      }
    */
#endif

      const auto half_tilesize = 0.5f * tilesize;

      std::unordered_set<glm::ivec2> n_gps;
      for (float x = x_min; x < x_max; x += tilesize) {
        for (float y = y_min; y < y_max; y += tilesize) {
          const glm::vec2 xy{ x, y };
          const glm::ivec2 gp = engine::grid::worldspace_to_gridspace(xy, tilesize);
          n_gps.emplace(gp);
        }
      }

      for (const auto n_gp : n_gps) {
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

        auto col = default_player_colours[player_c.colour_idx];

        draw_sprite(r,
                    Sprite{
                      .sprite = "CIRCLE",
                      .pos = n_pos,
                      .size = { 5, 5 },
                      .z_idx = ZLayer::FOREGROUND,
                      .col = col,
                    });

        // keep track of landable positions
        nearest_c.landable_positions.push_back({ island_e, n_pos });

        // if you collide with the island, land the player.
        // const auto* collided_with_island = r.try_get<PlayerCollidedWithIsland>(e);
        // const auto* immunity = r.try_get<IslandCollisionImmunity>(e);
        // if ((immunity && immunity->island_e == island_e) || !collided_with_island)
        //   continue;

        // no inputs.
        if (!has(input_c.button_s, ActionStateEnum::DOWN))
          continue;

        const auto boat_e = e;
        r.remove<PlayerCollidedWithIsland>(boat_e);

        land_player_on_island(r, island_c, n_gp, boat_e, island_e);
        nearest_c.landable_positions.clear();
        remove_hidden_state_from_island(r, island_e);

        // stop the boats momentum when you land.
        b2Body_SetLinearVelocity(r.get<PhysicsBodyComponent>(boat_e).bodyId, { 0, 0 });

        break; // give movement to one thing
      }
    }
  }
}

} // namespace game2d