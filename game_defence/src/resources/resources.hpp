#pragma once

#include "engine/audio/audio_components.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

SINGLE_RendererInfo
get_default_textures();

SINGLE_AudioComponent
get_default_audio();

} // namespace game2d