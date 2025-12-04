#include "pch.hpp"

#include "ui_survive_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_weapon_type_area/combat_weapon_type_area_components.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_upgrade_dodge/upgrade_dodge_components.hpp"
#include "modules/systems/system_upgrade_hp_regen/upgrade_hp_regen_components.hpp"
#include "modules/systems/system_upgrade_xp_zone_size/upgrade_xp_zone_size_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "ui_survive_upgrade_components.hpp"

namespace game2d {
using namespace std::literals;

#if defined(_DEBUG)
// static engine::RandomState roll_rnd(0); // same roll every time
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#else
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#endif

void
generate_upgrades_for_players(entt::registry& r, SINGLE_LevelUpUI& ui_c)
{
  int num_players = 4;

  for (int i = 0; i < num_players; i++) {
    auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;
    const auto& player_c = r.get<const PlayerBoatComponent>(player_e);

    auto fixture_e = get_fixture_by_tag(r, player_e, "fixture_player");
    if (!r.all_of<HealthComponent>(fixture_e))
      continue; // ur dead!

    UpgradeResultsComponent results_c;

    const auto get_rarity_from_roll = [](float roll) -> Rarity {
      int sum = 0;
      for (const auto [type, value] : rarity_chance_map) {
        sum += value;
        if (roll <= sum)
          return type;
      }
      return Rarity::COMMON; // default
    };

    const auto weapons_e = get_weapons(r, player_e);

    // get weapons that arnt max level (i.e. level 12)
    const int WEAPON_MAX_LEVEL = 12;
    std::vector<entt::entity> non_max_level_weapons;
    for (int i = 0; i < weapons_e.size(); i++) {
      const auto& wep_lvl_c = r.get<WeaponLevelComponent>(weapons_e[i]);
      const auto lv = wep_lvl_c.level;
      if (lv >= WEAPON_MAX_LEVEL)
        continue;
      non_max_level_weapons.push_back(weapons_e[i]);
    }

    // For the 1st & 2nd upgrade, roll a BULLET_X or WEAPON_X stat
    while (results_c.results.size() != 2) {

      // Prioritize non-max level weapons
      std::vector<entt::entity> weapons;
      if ((int)!non_max_level_weapons.empty()) {
        const int rnd_wep_upg_idx = engine::rand_det_s(roll_rnd.rng, 0, (int)non_max_level_weapons.size());
        const auto wep_e = non_max_level_weapons[rnd_wep_upg_idx];
        weapons.push_back(wep_e);
      } else if ((int)!weapons_e.empty()) {
        const int rnd_wep_upg_idx = engine::rand_det_s(roll_rnd.rng, 0, (int)weapons_e.size());
        const auto wep_e = weapons_e[rnd_wep_upg_idx];
        weapons.push_back(wep_e);
      }

      if (weapons.empty()) {
        throw std::runtime_error("player has no weapons!");
        exit(1); // crash
      }
      const auto weapon_type = r.get<Weapon_OnDiskData>(weapons[0]);

      // WEAPON_ stats, and add BULLET_ stats if applicable
      auto stats = upgradeable_weapon_stats;
      if (weapon_type.type_as_enum == WEAPON_TYPE::PROJECTILE || weapon_type.type_as_enum == WEAPON_TYPE::DEPLOY)
        stats.insert(stats.end(), upgradeable_bullet_stats.begin(), upgradeable_bullet_stats.end());

      if (weapon_type.damage_as_enum == WEAPON_DAMAGE::FIRE)
        stats.insert(stats.end(), upgradeable_area_stats.begin(), upgradeable_area_stats.end());

      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade_enum = stats[roll_value];
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));
      const auto [value, type] = get_stat_from_stat_table(r, rarity, upgrade_enum);

      results_c.results.emplace(UpgradeRollResult{
        .rarity = rarity,
        .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
        // WEAPON_x and BULLET_x do level weapon
        // .weapons = { weapon_e }, // note: only leveling first.
        .weapons = weapons,
        .level_weapons = true,
      });
    }

    // For the 3rd upgrade, roll an ACTOR_X stat.
    while (results_c.results.size() != 3) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)actor_x_stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade_enum = actor_x_stats[roll_value];
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));
      const auto [value, type] = get_stat_from_stat_table(r, rarity, upgrade_enum);

      results_c.results.emplace(UpgradeRollResult{
        .rarity = rarity,
        .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
        .weapons = {}, // ACTOR_x do not level weapon
        .level_weapons = false,
      });
    }

    r.emplace<UpgradeResultsComponent>(player_e, results_c);
  }

  SDL_Log("Generated upgrades for %i players", num_players);
};

void
update_player_upgrade_ui(entt::registry& r, entt::entity player_e, UIState& state_c)
{
  state_c.cells.clear();
  state_c.actions.clear();
  state_c.active = nullptr;

  // const auto& modifier_c = r.get<StatModifierComponent>(player_e); // check it has one
  const auto upgrades = find<UpgradeResultsComponent>(r, player_e);
  if (upgrades.empty())
    return;
  const auto& [upg_e, upg_c] = upgrades[0];

  for (const UpgradeRollResult& res : upg_c->results) {
    Cell c;
    c.name = ""; // replaced with the upgrade name when upgrade is populated
    c.action = [res, player_e]() {
      auto& evts_c = SINGLE_Events::instance;
      UpgradeEvent evt;
      evt.par_e = player_e;

      // upg_e is wep_e or par_e
      if (!res.weapons.empty())
        evt.upg_e = res.weapons[0];
      else
        evt.upg_e = player_e;

      evt.roll_result = res;
      evts_c.dispatcher->trigger(evt);
      evts_c.dispatcher->update();
    };
    state_c.cells.push_back(std::make_shared<Cell>(c));
  }

  // create navlinks
  // create_as_horizontal_layout(state_c.cells);
  create_as_vertical_layout(state_c.cells); // simple uses vertical layout

  // reset the selection
  state_c.active = state_c.cells[0];
}

void
populate_ui_based_on_upgrades(entt::registry& r, SINGLE_LevelUpUI& ui_c)
{
  SDL_Log("Populating upgrade ui...");
  const int max_num_players = 4;

  // reset ui
  for (int i = 0; i < max_num_players; i++) {

    const auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;

    update_player_upgrade_ui(r, player_e, ui_c.ui_states[i]);
  }
};

bool
is_choosing_upgrade(entt::registry& r)
{
  const auto view = r.view<UpgradeResultsComponent>();
  // for (const auto& [e, upg_c] : view.each()) {
  //   auto& tag_c = r.get<TagComponent>(e);
  //   SDL_Log("tag: %s", tag_c.tag.c_str());
  // }
  return !view.empty();
};

entt::entity
get_player_e_from_idx(entt::registry& r, const int player_idx)
{
  const auto player_es = view_to_vector_of_ents<PlayerBoatComponent>(r);

  const auto player_with_idx = [&r, player_idx](const entt::entity player_e) {
    auto& player_c = r.get<PlayerComponent>(player_e);
    return player_c.idx == player_idx;
  };

  const auto player_it = std::find_if(player_es.begin(), player_es.end(), player_with_idx);
  if (player_it == player_es.end())
    return entt::null; // player not joined

  return (*player_it);
};

SINGLE_UpgradeToName
load_upgrade_names(const std::string& path)
{
  SDL_Log("loading upgrade names... %s", path.c_str());

  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  SINGLE_UpgradeToName data = root.get<SINGLE_UpgradeToName>();

  // populate data stat_to_name_map
  for (const auto& upgrade_on_disk : data.names) {
    const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(upgrade_on_disk.stat).value();
    const UpgradeRollResult result{
      .rarity = magic_enum::enum_cast<Rarity>(upgrade_on_disk.rarity).value(),
      .stats = { Stat{ .stat = upgrade_on_disk.stat } },
    };
    data.stat_to_name_map[result] = upgrade_on_disk.name;
  }

  return data;
};

SINGLE_UpgradeToValue
load_upgrade_values(const std::string& path)
{
  SDL_Log("loading upgrade values... %s", path.c_str());

  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  SINGLE_UpgradeToValue data = root.get<SINGLE_UpgradeToValue>();

  return data;
};

std::string
make_stat_name_pretty_name(const std::string stat)
{
  std::string result = stat;

  // remove ACTOR_
  result = str_remove_all_occurances(result, "ACTOR_");
  result = str_remove_all_occurances(result, "AREA_");
  result = str_remove_all_occurances(result, "WEAPON_");
  // if (stat != "BULLET_SPEED")
  //   result = str_remove_all_occurances(result, "BULLET_");

  /**/
  // convert from bold uppercase to regular.
  // e.g. "WEAPON_RANGE" to "WEAPON RANGE"
  std::string res_str = "";
  for (const auto c : result) {
    if (c == '_')
      res_str.push_back(' '); // replace underscores
    else
      res_str.push_back(std::toupper(c));
  }

  return res_str;
};

std::string
get_val_str_from_stat_enum(entt::registry& r,
                           const entt::entity player_e,
                           const std::vector<entt::entity>& upg_weapons,
                           const UpgradeableStat stat_enum)
{
  const auto& actor_upgrades_c = r.get<StatModifierComponent>(player_e);
  const auto stat_str = std::string(magic_enum::enum_name(stat_enum));

  std::optional<float> value = std::nullopt;

  // DISPLAY ACTOR_ stats
  // assume no upg_weapons means ACTOR_ stat
  if (upg_weapons.empty()) {
    if (stat_enum == UpgradeableStat::ACTOR_DODGE_CHANCE) {
      const auto v = r.get<const ActorDodgeComponent>(player_e).dodge_percent;
      const auto v_out = actor_upgrades_c.apply_modifiers(v, stat_str);
      value = v_out;
    }
    if (stat_enum == UpgradeableStat::ACTOR_HEALTH_MAX) {
      const auto player_fixture_e = get_fixture_by_tag(r, player_e, "fixture_player");
      const auto v = r.get<const HealthComponent>(player_fixture_e).max_hp;
      const auto v_out = actor_upgrades_c.apply_modifiers(v, stat_str);
      value = v_out;
    }
    if (stat_enum == UpgradeableStat::ACTOR_HEALTH_REGEN) {
      const auto v = r.get<const ActorHealthRegenComponent>(player_e).hp_per_second;
      const auto v_out = actor_upgrades_c.apply_modifiers(v, stat_str);
      value = v_out;
    }
    if (stat_enum == UpgradeableStat::ACTOR_SPEED) {
      const auto v = r.get<const ActorSpeedComponent>(player_e).base_speed;
      const auto v_out = actor_upgrades_c.apply_modifiers(v, stat_str);
      value = v_out;
    }
    if (stat_enum == UpgradeableStat::ACTOR_XP_ZONE_SIZE) {
      const auto v = r.get<const ActorXpZoneSizeComponent>(player_e).radius_meters;
      const auto v_out = actor_upgrades_c.apply_modifiers(v, stat_str);
      value = v_out;
    }
  }

  // DISPLAY WEAPON_ and BULLET_ stats
  if (!upg_weapons.empty()) {
    const auto wep_e = upg_weapons[0];
    const auto wep_def = get_weapon_def(r, wep_e);
    const auto wep_data = r.get<Weapon_OnDiskData>(wep_e);
    const auto wep_type = wep_data.type_as_enum;

    if (wep_type == WEAPON_TYPE::PROJECTILE || wep_type == WEAPON_TYPE::DEPLOY) {
      const auto bul_def = get_bullet_def(r, wep_e);
      if (stat_enum == UpgradeableStat::BULLET_BOUNCE)
        value = (float)bul_def.bounces;
      else if (stat_enum == UpgradeableStat::BULLET_CRIT_CHANCE)
        value = (int)bul_def.crit_chance;
      else if (stat_enum == UpgradeableStat::BULLET_CRIT_DAMAGE)
        value = (int)bul_def.crit_damage;
      else if (stat_enum == UpgradeableStat::BULLET_DAMAGE)
        value = (float)bul_def.damage;
      else if (stat_enum == UpgradeableStat::BULLET_KNOCKBACK)
        value = bul_def.knockback_force; // mul x100 to make it more appealing
      else if (stat_enum == UpgradeableStat::BULLET_LIFESTEAL)
        value = (float)bul_def.lifesteal;
      else if (stat_enum == UpgradeableStat::BULLET_LIFETIME)
        value = (float)bul_def.lifecycle * 0.001f; // ms => s);
      else if (stat_enum == UpgradeableStat::BULLET_PIERCE)
        value = bul_def.pierce;
      else if (stat_enum == UpgradeableStat::BULLET_SIZE)
        value = (float)bul_def.size.x;
      else if (stat_enum == UpgradeableStat::BULLET_SPEED)
        value = (float)bul_def.speed;
    }

    // note: display the weapon that the upgrade is upgrading.
    if (stat_enum == UpgradeableStat::WEAPON_PROJECTILES)
      value = wep_def.projectiles;
    else if (stat_enum == UpgradeableStat::WEAPON_SPREAD)
      value = (float)wep_def.spread_deg;
    else if (stat_enum == UpgradeableStat::WEAPON_FIRERATE)
      value = (float)wep_def.fire_rate;
    else if (stat_enum == UpgradeableStat::WEAPON_CLIP_SIZE)
      value = wep_def.bullets_max;
    else if (stat_enum == UpgradeableStat::WEAPON_RELOAD)
      value = (float)wep_def.reload_rate;
    else if (stat_enum == UpgradeableStat::WEAPON_RANGE)
      value = (float)wep_def.range;
  }

  // DISPLAY AREA_ stats
  if (!upg_weapons.empty()) {
    const auto wep_e = upg_weapons[0];
    const auto wep_def = get_weapon_def(r, wep_e);
    const auto wep_data = r.get<Weapon_OnDiskData>(wep_e);
    const auto wep_type = wep_data.type_as_enum;
    const auto wep_damage = wep_data.damage_as_enum;

    if (wep_damage == WEAPON_DAMAGE::FIRE) {
      const auto area_def = get_area_def(r, wep_e);

      // if (stat_enum == UpgradeableStat::AREA_BEAMS_PER_WEAPON)
      //   val_str = std::format("{}", (int)area_def.beams);
      // else if (stat_enum == UpgradeableStat::AREA_SIZE)
      //   val_str = std::format("{} x {}", area_def.size_x, area_def.size_y);
      if (stat_enum == UpgradeableStat::AREA_STACK_DAMAGE)
        value = area_def.stack_damage;
      else if (stat_enum == UpgradeableStat::AREA_STACK_DURATION)
        value = area_def.stack_duration;
      else if (stat_enum == UpgradeableStat::AREA_STACKS_PER_SHOT)
        value = area_def.stacks_per_shot;
    }
  }

  if (!value.has_value())
    return "N/A";

  bool is_int = std::floor(value.value()) == value.value();
  if (is_int)
    return std::format("{}", (int)value.value());

  return std::format("{:.2f}", value.value());
};

} // namespace game2d