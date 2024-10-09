#pragma once

#include "generation/components.hpp"
#include "modules/map/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
generate_edges(entt::registry& r, MapComponent& map, const DungeonIntermediate& result);

void
generate_airlocks(entt::registry& r, MapComponent& map, const DungeonIntermediate& result);

void
instantiate_edge(entt::registry& r, entt::entity e, const MapComponent& map);

void
instantiate_edges(entt::registry& r, const MapComponent& map);

//
// Update pathfinding so that every "wall" that was generated
// the gridcell now only contains an entity with a pathfinding component
// that blocks off that cell witha value of -1 so it cant pathfind
//
void
update_map_with_pathfinding(entt::registry& r, MapComponent& map, DungeonIntermediate& result);

void
instantiate_floors(entt::registry& r, MapComponent& map, DungeonIntermediate& result);

} // namespace game2d