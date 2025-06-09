#pragma once

#include "engine/colour/colour.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace game2d {

struct SINGLE_UIScaling
{
  float scaling = 1.0f;
};

struct Cell
{
  std::string name;

  std::shared_ptr<Cell> l = nullptr;
  std::shared_ptr<Cell> r = nullptr;
  std::shared_ptr<Cell> u = nullptr;
  std::shared_ptr<Cell> d = nullptr;

  std::function<void()> action = []() {};

  virtual ~Cell() = default;
};

struct SelectableButtonDef
{
  std::string label;
  std::optional<std::string> icon = std::nullopt;
  ImVec2 size{ 20, 20 };
  bool input;

  std::shared_ptr<Cell>& cell;
  std::shared_ptr<Cell>& active_cell;

  // edge cases
  bool update_selected_only_with_mouse = false;
  bool update_selected_on_mouse_move = true;

  glm::vec2 text_pivot{ 0.5f, 0.5f }; // 0.5, 0.5 = center
  ImVec2 text_offset{ 0, 0 };
  ImFont* font = nullptr;

  engine::SRGBColour active_outline_col = hex_to_srgb("#FFFFFF", 255);
  engine::SRGBColour inactive_outline_col = hex_to_srgb("#FFFFFF", (int)(0.6f * 255));
  engine::SRGBColour active_bg_col = hex_to_srgb("#02526D", 255);
  engine::SRGBColour inactive_bg_col = hex_to_srgb("#02526D", (int)(0.6f * 255));
};

enum class UIAction
{
  NAV_MOVE_L,
  NAV_MOVE_R,
  NAV_MOVE_U,
  NAV_MOVE_D,

  SELECT,
  BACK,
};

// Note: this is limited.
// should support things like navigating left&right
struct UIState
{
  std::shared_ptr<Cell> active = nullptr;
  std::vector<std::shared_ptr<Cell>> cells;
  std::vector<UIAction> actions;
};

struct DefaultUI
{
  bool open = false;
  bool init = false;
  bool one_frame_buffer = true;
  UIState state;

  virtual void do_init(entt::registry& r) {};

  template<class T>
  std::optional<T> update(entt::registry& r)
  {
    std::optional<T> temp = std::nullopt;

    if (one_frame_buffer) {
      one_frame_buffer = false;
      return std::nullopt;
    }

    process_requests<T>(r, [&](const T& req) {
      open = true;
      temp = req; // note: if multiple reqs, uses last
    });

    return temp;
  }
};

} // namespace game2d