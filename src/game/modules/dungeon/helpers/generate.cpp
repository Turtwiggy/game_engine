#include "generate.hpp"

#include <vector>

namespace game2d {

EntityType
generate_item(const int floor, engine::RandomState& rnd)
{
  const std::vector<std::pair<int, std::pair<EntityType, int>>> difficulty_map{
    // clang-format off
    { 1, { EntityType::potion, 80 } },
    { 2, { EntityType::sword, 15 } },
    { 3, { EntityType::shield, 15 } },
    { 4, { EntityType::scroll_damage_nearest, 5 } },
    // { 1, { EntityType::scroll_damage_selected_on_grid, 5 } },
    // { 2, { EntityType::crossbow, 15 } },
    // { 2, { EntityType::bolt, 15 } },
    // clang-format on
  };

  std::map<EntityType, int> weights;
  for (const auto& [dict_floor, dict_type] : difficulty_map) {

    // only interested in weights at our floor
    if (dict_floor > floor)
      break;

    // sum the weights!
    const auto& entity_type = dict_type.first;
    const auto& entity_weight = dict_type.second;
    if (weights.contains(entity_type))
      weights[entity_type] += entity_weight;
    else
      weights[entity_type] = entity_weight;
  }

  int total_weight = 0;
  for (const auto& [k, w] : weights)
    total_weight += w;

  const float random = engine::rand_det_s(rnd.rng, 0, total_weight);
  int weight_acculum = 0;
  for (const auto& [k, w] : weights) {
    weight_acculum += w;
    if (weight_acculum >= random)
      return k;
  }
  return EntityType::potion;
};

EntityType
generate_monster(const int floor, engine::RandomState& rnd)
{
  const std::vector<std::pair<int, std::pair<EntityType, int>>> difficulty_map{
    // clang-format off
    { 1, { EntityType::actor_bat, 80 } },
    { 2, { EntityType::actor_troll, 15 } },
    { 3, { EntityType::actor_troll, 30 } },
    { 4, { EntityType::actor_troll, 60 } }
    // clang-format on
  };

  std::map<EntityType, int> weights;
  for (const auto& [dict_floor, dict_type] : difficulty_map) {

    // only interested in weights at our floor
    if (dict_floor > floor)
      break;

    // sum the weights!
    const auto& entity_type = dict_type.first;
    const auto& entity_weight = dict_type.second;
    if (weights.contains(entity_type))
      weights[entity_type] += entity_weight;
    else
      weights[entity_type] = entity_weight;
  }

  int total_weight = 0;
  for (const auto& [k, w] : weights)
    total_weight += w;

  const float random = engine::rand_det_s(rnd.rng, 0, total_weight);
  int weight_acculum = 0;
  for (const auto& [k, w] : weights) {
    weight_acculum += w;
    if (weight_acculum >= random)
      return k;
  }
  return EntityType::actor_bat;
};

} // namespace game2d