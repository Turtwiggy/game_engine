#include "pch.hpp"

#include "event_coll_player_tome_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_tome/tome_components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"

namespace game2d {

std::vector<Stat>
get_stats_from_weapon_behaviour(entt::registry& r, const WeaponBehaviour behaviour)
{
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  const auto behaviour_str = std::string(magic_enum::enum_name(behaviour));

  // search SINGLE_weapons for wep_behaviour stats
  std::vector<Stat> stats;
  for (const WeaponUpgrade_OnDiskData& upgrade : weapons_c.weapon_upgrades) {
    auto upg_str = upgrade.wb_key;
    auto upg_enum = magic_enum::enum_cast<WeaponBehaviour>(upg_str).value();
    if (behaviour != upg_enum)
      continue;
    for (const auto& stat_data : upgrade.stats)
      stats.push_back(Stat{ .stat = stat_data.stat, .type = stat_data.type, .value = stat_data.value });
  };

  return stats;
};

void
handle_player_enter_tome(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [pfixture_e, item_e] = coll<PlayerFixtureComponent, TomeComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with a tome!");

  static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  const auto& upgrades = weapons_c.weapon_upgrades;

  // generate some upgrades per player.
  for (const auto& [e, player_c] : r.view<PlayerBoatComponent>().each()) {
    auto weapons = get_weapons(r, e);
    const auto weapon_e = weapons[0];
    const auto& weapon_c = r.get<Weapon_OnDiskData>(weapon_e);
    const auto& behaviours_c = r.get<WeaponBehaviourComponent>(weapon_e);
    const auto& aquired_behaviours = behaviours_c.behaviours;

    std::vector<WeaponBehaviour> unaquired_wb;
    for (const auto& wb : upgrades) {
      auto it = std::find_if(aquired_behaviours.begin(), aquired_behaviours.end(), [&](const auto& wb_aquired) {
        const std::string key = wb.wb_key;
        const WeaponBehaviour wb_aquired_key = magic_enum::enum_cast<WeaponBehaviour>(key).value();
        return wb_aquired == wb_aquired_key;
      });
      if (it != aquired_behaviours.end())
        continue;
      const std::string key = wb.wb_key;
      const WeaponBehaviour wb_aquired_key = magic_enum::enum_cast<WeaponBehaviour>(key).value();

      // if it is limited to a specific weapon,
      // make sure we're using that weapon
      bool useable = true;
      for (const auto& limited_to : wb.limited_to)
        useable &= limited_to == weapon_c.key;
      if (!useable)
        continue;

      unaquired_wb.push_back(wb_aquired_key);
    }

    // offer the player one of them.
    const int n_upgrades = 3;

    UpgradeResultsComponent results_c;

    // let the player choose which upgrade to pick from upgrades you dont have.
    for (int i = 0; i < std::min(n_upgrades, (int)unaquired_wb.size()); i++) {
      const auto chosen_i = engine::rand_det_s(roll_rnd.rng, 0, (int)unaquired_wb.size());
      const auto wb_key = unaquired_wb[chosen_i];

      results_c.results.emplace(UpgradeRollResult{ .rarity = Rarity::COMMON,
                                                   .stats = get_stats_from_weapon_behaviour(r, wb_key),
                                                   .traits = { wb_key },
                                                   .weapons = weapons, // apply behaviour to all weapons
                                                   .level_weapons = false });

      std::erase(unaquired_wb, wb_key);
    }

    // if you collide with TWO tomes thats CRAZY.
    if (r.all_of<UpgradeResultsComponent>(e))
      SDL_Log("oops! you collided with a tome twice! (or while leveling up)");
    r.emplace_or_replace<UpgradeResultsComponent>(e, results_c);

    // update the ui.
    auto player_e = e;
    auto& ui_c = get_first_component<SINGLE_LevelUpUI>(r);
    auto player_idx = r.get<PlayerComponent>(player_e).idx;
    auto& state_c = ui_c.ui_states[player_idx];
    update_player_upgrade_ui(r, player_e, state_c);

    // destroy the tome
    auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
    dead_c.dead.push_back(item_e);
  }

  // play some audio.
  static engine::RandomState audio_rnd(0);
  const int rnd_audio = engine::rand_det_s(audio_rnd.rng, 1, 7);
  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "POSITIVE_0" + std::to_string(rnd_audio) });
}

} // namespace game2d