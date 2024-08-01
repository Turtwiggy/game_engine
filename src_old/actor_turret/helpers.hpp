#pragma once

#include <entt/entt.hpp>

#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <algorithm>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

namespace game2d {

[[nodiscard]] entt::entity
get_closest(entt::registry& r, const entt::entity e);

template<typename... Component>
[[nodiscard]] std::set<std::pair<entt::entity, int>>
get_within_range(entt::registry& r, const entt::entity e, const int d2_max)
{
  const auto& physics = get_first_component<const SINGLETON_PhysicsComponent>(r);
  const auto& t = r.get<TransformComponent>(e);

  std::optional<int> idx_x;
  std::optional<int> idx_y;

  // a.x_tl < b.x_tl;
  const auto it_x = std::find(physics.sorted_x.begin(), physics.sorted_x.end(), e);
  if (it_x != physics.sorted_x.end())
    idx_x = it_x - physics.sorted_x.begin();

  // a.y_tl < b.y_tl;
  const auto it_y = std::find(physics.sorted_y.begin(), physics.sorted_y.end(), e);
  if (it_y != physics.sorted_y.end())
    idx_y = it_y - physics.sorted_y.begin();

  if (!idx_x.has_value())
    return {};
  if (!idx_y.has_value())
    return {};

  const auto entity_in_range = [&r, &t, &d2_max](const entt::entity other) -> std::pair<bool, int> {
    // calculate distance
    const auto& other_pos = r.get<TransformComponent>(other);
    const auto d = t.position - other_pos.position;
    const int d2 = d.x * d.x + d.y * d.y;
    // const int d = int(glm::distance(glm::vec3(t.position), glm::vec3(other_pos.position)));
    return { d2 <= d2_max, d2 };
  };

  // Potential approaches for evaluating candidates.
  //
  // - continue iterating along and hope we find a candidate.
  // At worst-case, this would evaluate every entity (twice, once x-axis, once y-axis) in the physics system.
  //
  // - stop iterating immediately
  // This would guarentee that the next immediate neighbour is not the entity of interest.
  // but this is not particuly useful.
  // As this entity could be anything, this would make this function essentially useless.
  //
  // Solution:
  // present the algorithm a subset of interested entities, and maintain the sort order.
  //
  std::vector<entt::entity> entt_of_interest_on_the_left;
  std::vector<entt::entity> entt_of_interest_on_the_right;
  std::vector<entt::entity> entt_of_interest_above;
  std::vector<entt::entity> entt_of_interest_below;

  for (int i = idx_x.value() - 1; i >= 0; i--) {
    const auto& candidate = physics.sorted_x[i];
    const std::tuple<Component*...> comps = r.try_get<Component...>(candidate);

    bool all_valid = true; // iterate a tuple??
    std::apply([&all_valid](auto&&... args) { ((all_valid &= (args != nullptr)), ...); }, comps);
    if (all_valid)
      entt_of_interest_on_the_left.push_back(candidate);
  }
  for (int i = idx_x.value() + 1; i < physics.sorted_x.size(); i++) {
    const auto& candidate = physics.sorted_x[i];
    const std::tuple<Component*...> comps = r.try_get<Component...>(candidate);

    bool all_valid = true; // iterate a tuple??
    std::apply([&all_valid](auto&&... args) { ((all_valid &= (args != nullptr)), ...); }, comps);
    if (all_valid)
      entt_of_interest_on_the_right.push_back(candidate);
  }
  // check up... (y gets less)
  for (int i = idx_y.value() - 1; i >= 0; i--) {
    const auto& candidate = physics.sorted_y[i];
    const std::tuple<Component*...> comps = r.try_get<Component...>(candidate);

    bool all_valid = true; // iterate a tuple??
    std::apply([&all_valid](auto&&... args) { ((all_valid &= (args != nullptr)), ...); }, comps);
    if (all_valid)
      entt_of_interest_above.push_back(candidate);
  }
  // check down... (y gets greater)
  for (int i = idx_y.value() + 1; i < physics.sorted_y.size(); i++) {
    const auto& candidate = physics.sorted_y[i];
    const std::tuple<Component*...> comps = r.try_get<Component...>(candidate);

    bool all_valid = true; // iterate a tuple??
    std::apply([&all_valid](auto&&... args) { ((all_valid &= (args != nullptr)), ...); }, comps);
    if (all_valid)
      entt_of_interest_below.push_back(candidate);
  }
  //
  // Generate results based on filtered set.
  //
  std::set<std::pair<entt::entity, int>> results;
  for (const auto& e : entt_of_interest_on_the_left) {
    const auto result = entity_in_range(e);
    if (!result.first)
      break; // stop search. next entity only going to be furher away
    results.emplace(std::make_pair(e, result.second));
  }
  for (const auto& e : entt_of_interest_on_the_right) {
    const auto result = entity_in_range(e);
    if (!result.first)
      break; // stop search. next entity only going to be furher away
    results.emplace(std::make_pair(e, result.second));
  }
  for (const auto& e : entt_of_interest_above) {
    const auto result = entity_in_range(e);
    if (!result.first)
      break; // stop search. next entity only going to be furher away
    results.emplace(std::make_pair(e, result.second));
  }
  for (const auto& e : entt_of_interest_below) {
    const auto result = entity_in_range(e);
    if (!result.first)
      break; // stop search. next entity only going to be furher away
    results.emplace(std::make_pair(e, result.second));
  }

  return results;
};

} // namespace game2d