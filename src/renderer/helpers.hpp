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

TextureUnit
request_texture(std::vector<Texture>& textures,
                const std::optional<std::string>& path = std::nullopt,
                const std::optional<std::string>& spritesheet = std::nullopt);

void
rebind(const SINGLETON_RendererInfo& ri, const std::vector<Texture>& tex, const glm::ivec2& wh);

void
check_if_viewport_resize(SINGLETON_RendererInfo& ri, std::vector<Texture>& tex, glm::ivec2& viewport_wh);

}; // namespace game2d