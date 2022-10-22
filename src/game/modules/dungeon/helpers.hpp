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

entt::entity
create_dungeon_entity(GameEditor& editor, entt::registry& r, EntityType et, const glm::ivec2& grid_index);

constexpr glm::ivec2
room_center(const Room& r)
{
  return { (r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2 };
};

bool
rooms_overlap(const Room& r0, const Room& r1);

void
create_room(GameEditor& editor, entt::registry& r, const Room& room);

void
create_tunnel_floor(GameEditor& editor, entt::registry& r, const Dungeon& d, std::vector<std::pair<int, int>>& coords);

// Create an L-shaped tunnel between two points
void
create_tunnel(GameEditor& editor, entt::registry& r, const Dungeon& d, int x1, int y1, int x2, int y2);

//
// Gameplay logic below
//

void
set_pathfinding_cost(GameEditor& editor, entt::registry& r);

void
set_player_positions(GameEditor& editor, entt::registry& r, std::vector<Room>& rooms, engine::RandomState& rnd);

void
set_generated_entity_positions(GameEditor& editor,
                               entt::registry& r,
                               std::vector<Room>& rooms,
                               const int floor,
                               engine::RandomState& rnd);

} // namespace game2d