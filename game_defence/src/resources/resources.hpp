#pragma once

#include "engine/audio/audio_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/system_quips/components.hpp"

namespace game2d {

SINGLE_RendererInfo
get_default_textures();

SINGLE_AudioComponent
get_default_audio();

SINGLE_QuipsComponent
get_default_quips();

SINGLE_NamesComponent
get_default_names();

} // namespace game2d