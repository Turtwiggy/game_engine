#pragma once

#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_components.hpp"

#include <string>
#include <vector>

namespace game2d {
using namespace std::literals;

struct HullChoice
{
  bool confirmed = false;

  float confirm_held_time = 0.0f;
  float confirm_held_time_max = 0.6f;

  float back_held_time = 0.0f;
  float back_held_time_max = 1.1f;

  int player_row_idx = 0;
  int player_idx = 0;
  int player_colour_idx = 0;

  std::string player_boat_key = "unknown";
  std::string player_gun_key = "unknown";
  std::string player_ability_key = "unknown";
  std::string player_name = "unknown";
  bool autofire = true;
};

struct SelectUI : public DefaultUI
{
  int select_choice_idx = 0;
};

struct SINGLE_SelectSceneData : public DefaultUI
{
#if defined(_DEBUG)
  float countdown_max = 0.0f;
  float countdown = countdown_max;
#else
  float countdown_max = 0.0f;
  float countdown = countdown_max;
#endif

  std::vector<SelectUI> player_ui_state;
  std::vector<HullChoice> player_choice_state;
  std::vector<UiCursorComponent> player_cursor_state;
  std::vector<std::string> available_names;

  std::vector<ShipHullData> unlocked_hulls;
  std::vector<Weapon_OnDiskData> unlocked_weapons;
};

struct SelectSceneToSurviveScene
{
  std::vector<HullChoice> chosen_boats;
};

struct DisplayStat
{
  std::string key;
  std::string val;
  std::optional<UpgradeableStat> stat = std::nullopt;
};

} // namespace game2d