#pragma once

#include "modules/core/options/options_components.hpp"
#include "modules/core/ui/ui_common_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

enum class MODIFIER_OPTIONS
{
  ROCKS,

  count
};

struct IModifierOption
{
  MODIFIER_OPTIONS option;
  std::string display_str;
  UIValueType value_type;

  IModifierOption() = delete;
  IModifierOption(const MODIFIER_OPTIONS in_option, const std::string in_display, const UIValueType type)
    : option(in_option)
    , display_str(in_display)
    , value_type(type) {};

  virtual void update(entt::registry& r, int& hindex) {};
};

struct Option_Rocks : public IModifierOption
{
  // data
  bool populate_rocks = false;

  Option_Rocks()
    : IModifierOption(MODIFIER_OPTIONS::ROCKS, "Rocks?", UIValueType::BUTTON) {};

  void update(entt::registry& r, int& hindex) override;
};

// Persistent data for the modifiers
struct SINGLE_ModifiersData
{
  std::vector<std::shared_ptr<IModifierOption>> options{
    //
    std::make_shared<Option_Rocks>(),
    //
  };
};

// UI data for the modifiers screen
struct SINGLE_UISelectModifiersMenuState
{
  bool open = false;
  bool init = false;
  bool one_frame_buffer = true;

  UIState state;
};

struct RequestToShowModifierMenu
{
  bool placeholder = true;
};

} // namespace game2d