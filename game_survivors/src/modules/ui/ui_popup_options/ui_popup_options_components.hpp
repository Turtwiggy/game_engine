#pragma once

#include "engine/app/game_window.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include <entt/fwd.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct RequestToShowOptionsMenu
{
  bool placeholder = true;
};

struct OptionsCell : public Cell
{
  int value = 0;
};

struct SINGLE_OptionsMenuState : DefaultUI
{
  bool init = false;

  void do_init(entt::registry& r);
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

struct Video_WhichMonitorOnDisk
{
  int monitor_idx = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Video_WhichMonitorOnDisk, monitor_idx);
};

} // namespace game2d