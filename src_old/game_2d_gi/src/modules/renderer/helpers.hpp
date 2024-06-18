#pragma once

// game2d
#include "components.hpp"

// other libs
#include <imgui.h>

#include <optional>
#include <vector>

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

bool
check_if_viewport_resize(const SINGLETON_RendererInfo& ri);

}; // namespace game2d