#include "pch.hpp"

#include "ui_survive_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "ui_survive_upgrade_components.hpp"

namespace game2d {

#if defined(_DEBUG)
// static engine::RandomState roll_rnd(0); // same roll every time
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#else
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#endif

void
generate_upgrades_for_players(entt::registry& r, SINGLE_LevelUpUI& ui_c)
{
  int gen_count = 0;

  for (int i = 0; i < 4; i++) {
    auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;
    const auto& player_c = r.get<PlayerComponent>(player_e);

    UpgradeResultsComponent results_c;

    // Roll 3 times for 3 upgrades.
    for (int upg_idx = 0; upg_idx < 3; upg_idx++) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)traits_to_level_up.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      Rarity rarity = Rarity::COMMON;
      int sum = 0;
      for (auto [type, value] : rarity_chance_map) {
        sum += value;
        if (roll_rarity <= sum) {
          rarity = type;
          break;
        }
      }

      const auto upgrade = traits_to_level_up[roll_value];
      results_c.results.push_back({ .rarity = rarity, .upgrade = upgrade });
    }

    gen_count++;
    r.emplace<UpgradeResultsComponent>(
      player_e, UpgradeResultsComponent{ .results = std::vector(results_c.results.begin(), results_c.results.end()) });
  }

  SDL_Log("Generated upgrades for %i players", gen_count);
};

bool
is_choosing_upgrade(entt::registry& r)
{
  return r.view<UpgradeResultsComponent>().size() > 0;
};

entt::entity
get_player_e_from_idx(entt::registry& r, int player_idx)
{
  const auto player_es = view_to_vector_of_ents<PlayerComponent>(r);

  auto player_with_idx = [&r, player_idx](const entt::entity player_e) {
    auto& player_c = r.get<PlayerComponent>(player_e);
    return player_c.idx == player_idx;
  };

  auto player_it = std::find_if(player_es.begin(), player_es.end(), player_with_idx);
  if (player_it == player_es.end())
    return entt::null; // player not joined

  return (*player_it);
};

} // namespace game2d