#pragma once

#include "game/components/actors.hpp"
#include "game/components/app.hpp"
#include "game/modules/dungeon/components.hpp"
#include "engine/maths/maths.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

entt::entity
create_dungeon_entity(GameEditor& editor, Game& game, EntityType et, const glm::ivec2& grid_index);

void
create_all_walls(GameEditor& editor, Game& game, Dungeon& d);

void
create_all_rooms(GameEditor& editor, Game& game, Dungeon& d, engine::RandomState rnd);

void
create_room(GameEditor& editor, Game& game, Dungeon& d, const Room& room, int room_index);

void
create_tunnel(GameEditor& editor, Game& game, Dungeon& d, int x1, int y1, int x2, int y2);

} // namespace game2d