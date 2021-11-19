#pragma once

// game2d
#include "modules/renderer/components.hpp"

namespace game2d {

struct ViewportInfo
{
  ImVec2 pos;
  ImVec2 size;
  bool focused = false;
  bool hovered = false;
};

ViewportInfo
render_texture_to_imgui_viewport(const int& tex_unit);

}; // namespace game2d