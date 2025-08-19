#pragma once

#include "engine/app/application.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

enum class GAME_OPTIONS
{
  AUDIO_MASTER_VOLUME,
  AUDIO_MUSIC_VOLUME,
  AUDIO_SFX_VOLUME,

  VIDEO_SCREEN_MODE,
  VIDEO_RESOLUTION,
  VIDEO_VSYNC,
  VIDEO_WHICH_MONITOR,

  count
};

// NOTE: this currently isnt used
enum class UIValueType
{
  BUTTON,
  SCROLL,
  SELECTION,
};

struct IOption
{
  GAME_OPTIONS option;
  std::string display_str;
  bool loaded = false;
  UIValueType value_type;

  IOption() = delete;
  IOption(const GAME_OPTIONS in_option, const std::string in_display, const UIValueType type)
    : option(in_option)
    , display_str(in_display)
    , value_type(type) {};

  // fun-ctions, because they're fun. right. right?
  virtual void load(engine::SINGLE_Application& app, entt::registry& r) {};
  virtual void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) {};
  virtual int get_hindex(entt::registry& r) { return 0; };

  // display the option value to the user
  virtual std::string display_val() { return ""; };
};

struct Option_AudioMasterVolume : public IOption
{
  Audio_OnDisk data;

  Option_AudioMasterVolume()
    : IOption(GAME_OPTIONS::AUDIO_MASTER_VOLUME, "Master", UIValueType::SCROLL) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct Option_AudioMusicVolume : public IOption
{
  Audio_OnDisk data;

  Option_AudioMusicVolume()
    : IOption(GAME_OPTIONS::AUDIO_MUSIC_VOLUME, "Music", UIValueType::SCROLL) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct Option_AudioSFXVolume : public IOption
{
  Audio_OnDisk data;

  Option_AudioSFXVolume()
    : IOption(GAME_OPTIONS::AUDIO_SFX_VOLUME, "SFX", UIValueType::SCROLL) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct Option_VideoScreenMode : public IOption
{
  Video_ScreenModeOnDisk data;

  Option_VideoScreenMode()
    : IOption(GAME_OPTIONS::VIDEO_SCREEN_MODE, "Screen Mode", UIValueType::SELECTION) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct Option_VideoResolution : public IOption
{
  Video_ResolutionOnDisk data;

  Option_VideoResolution()
    : IOption(GAME_OPTIONS::VIDEO_RESOLUTION, "Resolution", UIValueType::SELECTION) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct Option_VideoVsync : public IOption
{
  Video_VsyncOnDisk data;

  Option_VideoVsync()
    : IOption(GAME_OPTIONS::VIDEO_VSYNC, "Vsync", UIValueType::BUTTON) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct Option_VideoWhichMonitor : public IOption
{
  Video_WhichMonitorOnDisk data;

  Option_VideoWhichMonitor()
    : IOption(GAME_OPTIONS::VIDEO_WHICH_MONITOR, "Monitor", UIValueType::SELECTION) {};

  void load(engine::SINGLE_Application& app, entt::registry& r) override;
  void update(engine::SINGLE_Application& app, entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;
  std::string display_val() override;
};

struct SINGLE_GameOptions
{
  bool loaded = false;

  std::vector<std::shared_ptr<IOption>> options{
    // audio settings
    std::make_shared<Option_AudioMasterVolume>(),
    std::make_shared<Option_AudioMusicVolume>(),
    std::make_shared<Option_AudioSFXVolume>(),
    // video settings
    std::make_shared<Option_VideoScreenMode>(),
    std::make_shared<Option_VideoResolution>(),
    std::make_shared<Option_VideoVsync>(),
    std::make_shared<Option_VideoWhichMonitor>(),
  };
};

} // namespace game2d