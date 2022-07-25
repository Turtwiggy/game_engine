#pragma once

// game2d
#include "modules/renderer/components.hpp"

// other libs
#include <imgui.h>

namespace game2d {

struct ViewportInfo
{
  ImVec2 pos;
  ImVec2 size;
  bool focused = false;
  bool hovered = false;
};

ViewportInfo
render_texture_to_imgui_viewport(const int64_t& tex_unit);

}; // namespace game2d