#pragma once

#include "modules/ui_common/ui_common_components.hpp"
#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

enum class GAME_OPTIONS
{
  AUDIO_MASTER_VOLUME,
  AUDIO_MUSIC_VOLUME,
  AUDIO_SFX_VOLUME,

  VIDEO_SCREEN_MODE,
  VIDEO_RESOLUTION,
  VIDEO_VSYNC,

  count
};

struct RequestToShowOptionsMenu
{
  bool placeholder = true;
};

struct SINGLE_OptionsMenuState
{
  bool open = false;
  bool init = false;
  bool one_frame_buffer = true;

  UIState state;
};

// VIDEO_SCREEN_MODE,
// VIDEO_RESOLUTION,
// VIDEO_VSYNC,

struct AudioMaster_OnDisk
{
  float value = 1.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(AudioMaster_OnDisk, value);
};
struct AudioMusic_OnDisk
{
  float value = 1.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(AudioMusic_OnDisk, value);
};
struct AudioSFX_OnDisk
{
  float value = 1.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(AudioSFX_OnDisk, value);
};

} // namespace game2d