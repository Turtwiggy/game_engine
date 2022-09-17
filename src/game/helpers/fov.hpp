#pragma once

#include <entt/entt.hpp>

namespace game2d {

// light colours
struct VisibleComponent
{
  bool placeholder = true;
};

// shroud colours
struct NotVisibleComponent
{
  bool placeholder = true;
};

// dark colours
struct NotVisibleButPreviouslySeenComponent
{
  bool placeholder = true;
};

// struct TileBlocksFoVComponent
// {
//   bool placeholder = true;
// };

// struct SpriteTagComponent
// {
//   std::string tag;
//   SpriteComponent sprite;
// };

// struct SpriteSwapBasedOnStateComponent
// {
//   std::vector<SpriteTagComponent> sprites;
// };

} // game2d