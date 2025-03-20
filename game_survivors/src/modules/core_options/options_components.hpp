#pragma once

#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct IOption
{
  GAME_OPTIONS option;
  std::string display_str;

  IOption() = delete;
  IOption(const GAME_OPTIONS in_option, const std::string in_display)
    : option(in_option)
    , display_str(in_display) {};

  // fun-ctions, because they're fun. right. right?
  virtual void load(entt::registry& r) {};
  virtual void update(entt::registry& r, int& hindex) {};
  virtual int get_hindex(entt::registry& r) { return 0; };

  // display the option value to the user
  virtual std::string display_val() { return ""; };
};

struct Option_AudioMasterVolume : public IOption
{
  float val = 1.0f;

  Option_AudioMasterVolume()
    : IOption(GAME_OPTIONS::AUDIO_MASTER_VOLUME, "Master") {};

  void load(entt::registry& r) override;
  void update(entt::registry& r, int& hindex) override;
  int get_hindex(entt::registry& r) override;

  std::string display_val() override;
};

struct SINGLE_GameOptions
{
  std::vector<std::shared_ptr<IOption>> options{
    std::make_shared<Option_AudioMasterVolume>(),
  };
};

} // namespace game2d