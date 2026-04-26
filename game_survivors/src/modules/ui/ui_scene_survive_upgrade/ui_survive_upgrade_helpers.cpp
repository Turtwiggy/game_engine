#include "pch.hpp"

#include "ui_survive_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/std/string/helpers.hpp"
#include "engine/std/vector/helpers.hpp"
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
#include "modules/systems/system_shoot_auto/autofire_helpers.hpp"
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

const auto get_rarity_from_roll = [](float roll) -> Rarity {
  int sum = 0;
  for (const auto [type, value] : rarity_chance_map) {
    sum += value;
    if (roll <= sum)
      return type;
  }
  return Rarity::COMMON; // default
};

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

    const auto weapons_e = get_weapons(r, player_e);
    auto& stat_c = r.get<StatModifierComponent>(player_e);
    auto& wep_stat_c = r.get<StatModifierComponent>(weapons_e[0]);
    // assume player has the same type of weapons in all weapon slots
    const auto weapon_type = r.get<Weapon_OnDiskData>(weapons_e[0]);

    UpgradeResultsComponent results_c;

    std::vector<UpgradeableStat> stats;
    {
      // WEAPON_ stats
      stats.insert(stats.end(), upgradeable_weapon_stats.begin(), upgradeable_weapon_stats.end());

      // ACTOR_ stats.
      stats.insert(stats.end(), actor_x_stats.begin(), actor_x_stats.end());

      // BULLET_ stats (if applicable)
      if (weapon_type.type_as_enum == WEAPON_TYPE::PROJECTILE || weapon_type.type_as_enum == WEAPON_TYPE::DEPLOY)
        stats.insert(stats.end(), upgradeable_bullet_stats.begin(), upgradeable_bullet_stats.end());

      // AREA_ stats if the bullet is elemental
      auto& behaviours_c = r.get<WeaponBehaviourComponent>(weapons_e[0]);
      bool is_fire = weapon_type.damage_as_enum == WEAPON_DAMAGE::FIRE;
      // is_fire |= has(behaviours_c.behaviours, WeaponBehaviour::CHANGE_DAMAGE_TO_FIRE);
      if (is_fire)
        stats.insert(stats.end(), upgradeable_area_stats.begin(), upgradeable_area_stats.end());
    }

    // Figure out the stats the player already has maxed out.
    std::unordered_set<UpgradeableStat> maxed_out_stats;
    for (const auto& stat : stats) {
      const auto upgrade_enum = stat;
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));

      // Check how many of the stats we have
      const auto cmp = [&](const auto& modifier) { return modifier->stat == upgrade_str; };
      const int actor_occurances = std::count_if(stat_c.modifiers.begin(), stat_c.modifiers.end(), cmp);
      const int weapon_occurances = std::count_if(wep_stat_c.modifiers.begin(), wep_stat_c.modifiers.end(), cmp);
      const int n_stat = actor_occurances + weapon_occurances;
      SDL_Log("You have: %i %s", n_stat, upgrade_str.c_str());

      // only 5 levels per stat
      if (n_stat >= 5) {
        maxed_out_stats.insert(upgrade_enum);
        continue;
      }
    }

    // remove the stats that are already maxed out
    for (const auto stat : maxed_out_stats)
      stats.erase(std::remove(stats.begin(), stats.end(), stat), stats.end());

    for (int u = 0; u < 3; u++) {

      // oops! cant generate any more stats. what to offer player now?
      if (stats.empty()) {
        SDL_Log("You're out of upgrades!");

        // for the moment, just add a blank upgrade to get out of the while loop
        results_c.results.emplace(UpgradeRollResult{
          .rarity = Rarity::COMMON,
          .stats = { Stat{ .stat = "BULLET_DAMAGE", .type = "stat_flat_increase", .value = 0 } },
          .weapons = {},
        });

        if (results_c.results.size() == 3)
          break;

        continue;
      }

      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)stats.size());
      const auto upgrade_enum = stats[roll_value];
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));

      // dont roll the same stat twice, it feels bad.
      stats.erase(std::remove(stats.begin(), stats.end(), upgrade_enum), stats.end());

      // Check how many of the stats we have
      const auto cmp = [&](const auto& modifier) { return modifier->stat == upgrade_str; };
      const int actor_occurances = std::count_if(stat_c.modifiers.begin(), stat_c.modifiers.end(), cmp);
      const int weapon_occurances = std::count_if(wep_stat_c.modifiers.begin(), wep_stat_c.modifiers.end(), cmp);
      const int n_stat = actor_occurances + weapon_occurances;
      SDL_Log("You have: %i %s", n_stat, upgrade_str.c_str());

      const auto rarity = (Rarity)(n_stat + 1);
      const auto [value, type] = get_stat_from_stat_table(r, rarity, upgrade_enum);

      // actor_ stats dont apply to weapons
      const bool is_actor_stat = upgrade_str.find("ACTOR_") != std::string::npos;
      auto weapons_to_upg = std::vector<entt::entity>();
      if (!is_actor_stat)
        weapons_to_upg = weapons_e;

      results_c.results.emplace(UpgradeRollResult{
        .rarity = rarity,
        .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
        // WEAPON_x and BULLET_x do level weapon
        // .weapons = { weapon_e }, // note: only leveling first.
        .weapons = weapons_to_upg,
        .level_weapons = true,
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

  if (upg_c->results.empty()) {
    r.remove<UpgradeResultsComponent>(upg_e);
    return;
  }

  for (const UpgradeRollResult& res : upg_c->results) {
    Cell c;
    c.name = ""; // replaced with the upgrade name when upgrade is populated
    c.action = [res, player_e]() {
      auto& evts_c = SINGLE_Events::instance;
      UpgradeEvent evt;
      evt.par_e = player_e;

      // upg_e is wep_e or par_e
      if (!res.weapons.empty())
        evt.upg_es = res.weapons;
      else
        evt.upg_es = { player_e };

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
  if (!state_c.cells.empty())
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
    const auto& wep_behaviours_c = r.get<WeaponBehaviourComponent>(wep_e);

    bool is_fire = wep_damage == WEAPON_DAMAGE::FIRE;

    // note: weapons that get CHANGE_DAMAGE_TO_FIRE dont have an area_def
    // is_fire |= has(wep_behaviours_c.behaviours, WeaponBehaviour::CHANGE_DAMAGE_TO_FIRE);

    if (is_fire) {
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

  auto stat_amount = std::format("{:.2f}", value.value());

  // if the last digit is a 0, remove it
  if (stat_amount.find(".") != std::string::npos && stat_amount.back() == '0')
    stat_amount.pop_back();

  return stat_amount;
};

} // namespace game2d