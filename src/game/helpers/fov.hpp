#pragma once

namespace game2d {

enum class TileState : unsigned char
{
  visible,
  not_visible,
  not_visible_but_seen_before,
};

struct VisibleComponent
{
  bool placeholder = true;
};

struct NotVisibleComponent
{
  bool placeholder = true;
};

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