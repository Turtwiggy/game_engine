#pragma once

#include <string>
#include <vector>

namespace game2d {
using namespace std::literals;

struct HullChoice
{
  int player_handle = 0;
  int idx = 0;
  bool confirmed = false;
};

struct SINGLE_SelectSceneData
{
  // wait one frame before processing scene,
  // otherwise the input that sent you
  // to the scene will move you to the next scene.
  bool menu_to_select_scene_buffer_frame = true;

  float countdown_max = 3.0f;
  float countdown = 3.0f;

  std::vector<HullChoice> player_index_to_hull;
};

struct SelectSceneToSurviveScene
{
  std::string chosen_boat = "Dinghy"s;
};

} // namespace game2d