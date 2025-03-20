#pragma once

#include "engine/app/game_window.hpp"
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

//
// on-disk data representations
//

struct Audio_OnDisk
{
  float value = 1.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Audio_OnDisk, value);
};

struct Video_ScreenModeOnDisk
{
  engine::DisplayMode screen_mode = engine::DisplayMode::windowed_borderless;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Video_ScreenModeOnDisk, screen_mode);
};

struct Video_ResolutionOnDisk
{
  int w = 1920;
  int h = 1080;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Video_ResolutionOnDisk, w, h);
};

struct Video_VsyncOnDisk
{
  bool enabled = true;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Video_VsyncOnDisk, enabled);
};

} // namespace game2d