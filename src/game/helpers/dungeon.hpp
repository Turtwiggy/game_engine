#pragma once

#include "engine/maths/grid.hpp"
#include "game/entities/actors.hpp"

#include <entt/entt.hpp>

#include <map>

//
// lots of placeholder work
//

namespace game2d {

struct TileBlocksFoVComponent
{
  bool placeholder = true;
};

struct SpriteTagComponent
{
  std::string tag;
  SpriteComponent sprite;
};

struct SpriteSwapBasedOnStateComponent
{
  std::vector<SpriteTagComponent> sprites;
};

using Coord = std::pair<int, int>;
using Entities = std::vector<entt::entity>;

struct Dungeon
{
  int width = 30;
  int height = 30;

  std::map<Coord, Entities> tilemap;
};

struct Room
{
  bool placeholder = true;
};

void
generate_dungeon(entt::registry& r, Dungeon& d);

} // namespace game2d