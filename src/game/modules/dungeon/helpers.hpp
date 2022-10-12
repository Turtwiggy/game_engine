#pragma once

#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/components/app.hpp"
#include "game/modules/dungeon/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

std::vector<entt::entity>
grid_entities_at(entt::registry& r, int x, int y);

// e.g. 0, 0 should return {1, 0}, {1, 1}, {0, 1}
void
get_neighbour_indicies(const int x,
                       const int y,
                       const int x_max,
                       const int y_max,
                       std::vector<std::pair<GridDirection, int>>& results);

constexpr glm::ivec2
room_center(const Room& r)
{
  return { (r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2 };
};

bool
rooms_overlap(const Room& r0, const Room& r1);

void
create_room(GameEditor& editor, Game& game, const Room& room);

void
create_tunnel_floor(GameEditor& editor, Game& game, const Dungeon& d, std::vector<std::pair<int, int>>& coords);

// Create an L-shaped tunnel between two points
void
create_tunnel(GameEditor& editor, Game& game, const Dungeon& d, int x1, int y1, int x2, int y2);

//
// Gameplay logic below
//

entt::entity
create_dungeon_entity(GameEditor& editor, Game& game, EntityType et, glm::ivec2 grid_index);

void
set_pathfinding_cost(GameEditor& editor, Game& game);

void
set_player_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd);

void
set_enemy_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd);

void
set_item_positions(GameEditor& editor, Game& game, std::vector<Room>& rooms, engine::RandomState& rnd);

} // namespace game2d