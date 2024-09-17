#pragma once

// game2d
#include "components.hpp"
#include "modules/renderer/components.hpp"

// other libs
#include <imgui.h>

#include <optional>
#include <string>

namespace game2d {

ViewportInfo
render_texture_to_imgui_viewport(const int64_t& tex_unit);

bool
check_if_viewport_resize(const SINGLE_RendererInfo& ri);

//

std::optional<TextureUnit>
search_for_texture_unit_by_texture_path(const SINGLE_RendererInfo& ri, const std::string& search);

std::optional<TextureId>
search_for_texture_id_by_texture_path(const SINGLE_RendererInfo& ri, const std::string& search);

std::optional<TextureUnit>
search_for_texture_unit_by_spritesheet_path(const SINGLE_RendererInfo& ri, const std::string& search);

std::optional<TextureId>
search_for_texture_id_by_spritesheet_path(const SINGLE_RendererInfo& ri, const std::string& search);

//

int
search_for_renderpass_by_name(const SINGLE_RendererInfo& ri, const PassName& name);

}; // namespace game2d