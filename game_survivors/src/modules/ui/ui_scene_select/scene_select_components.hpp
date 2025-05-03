#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include <string>
#include <vector>

namespace game2d {
using namespace std::literals;

struct HullChoice
{
  bool confirmed = false;

  int player_row_idx = 0;
  int player_idx = 0;
  std::string player_boat_key = "unknown";
  std::string player_gun_key = "unknown";
  std::string player_ability_key = "unknown";
};

struct SelectUI : public DefaultUI
{
  int cell_idx = 0;
};

struct SINGLE_SelectSceneData
{
  // wait one frame before processing scene,
  // otherwise the input that sent you
  // to the scene will move you to the next scene.
  bool menu_to_select_scene_buffer_frame = true;

#if defined(_DEBUG)
  float countdown_max = 0.5f;
  float countdown = 0.5f;
#else
  float countdown_max = 3.0f;
  float countdown = 3.0f;
#endif

  std::vector<SelectUI> player_ui_state;
  std::vector<HullChoice> player_choice_state;
};

struct SelectSceneToSurviveScene
{
  std::vector<HullChoice> chosen_boats;
};

} // namespace game2d