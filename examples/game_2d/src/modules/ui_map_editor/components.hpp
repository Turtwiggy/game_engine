#pragma once

// game headers
#include "helpers/spritemap.hpp"

// engine headers
#include "engine/opengl/shader.hpp"

// c++ lib headers
#include <optional>
#include <vector>

namespace game2d {

struct SINGLETON_MapEditorInfo
{
  bool place_sprite = false;
  std::optional<sprite::type> sprite_to_place = std::nullopt;
};

} // namespace game2d