#pragma once

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_enemy_patrol/system.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/grid/components.hpp"
#include "physics/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <gtest/gtest.h>

namespace game2d {

namespace tests {

TEST(TestSuite, ActorEnemyPatrol_Backstabs)
{
  entt::registry r;

  // create a map
  int map_width = 100;
  int map_height = 100;
  MapComponent map_c;
  map_c.tilesize = 10;
  map_c.xmax = map_width / map_c.tilesize;
  map_c.ymax = map_height / map_c.tilesize;
  map_c.map.resize(map_c.xmax * map_c.ymax);

  auto map_e = create_empty<MapComponent>(r);
  r.emplace<MapComponent>(map_e, map_c);

  const auto player_e = create_gameplay(r, EntityType::actor_player);

  int i = 0;
  const auto test_patrol =
    [&r, &player_e, &i](const glm::vec2& lookdir, const glm::ivec2& player_pos, const bool is_backstabbable) {
      std::cout << "Testing patrol case: " << i++ << std::endl;

      // Arrange
      const auto patrol_e = create_gameplay(r, EntityType::actor_enemy_patrol);
      r.remove<WaitForInitComponent>(patrol_e); // init immediate
      r.emplace_or_replace<VelocityComponent>(patrol_e, lookdir.x, lookdir.y);
      set_position(r, player_e, player_pos);

      // Act
      update_actor_enemy_patrol_system(r, glm::ivec2(0, 0), 0);

      // Assert
      const auto backstab = r.try_get<BackstabbableComponent>(patrol_e);
      const bool backstab_component_added = backstab != nullptr;
      ASSERT_TRUE(backstab_component_added == is_backstabbable);

      // Cleanup
      r.destroy(patrol_e);
    };

  const glm::vec2 looking_right = { 1, 0 };
  const glm::vec2 looking_left = { -1, 0 };
  const glm::vec2 looking_up = { 0, -1 };
  const glm::vec2 looking_down = { 0, 1 };

  const glm::ivec2 on_right = { 10, 0 };
  const glm::ivec2 on_left = { -10, 0 };
  const glm::ivec2 above = { 0, -10 };
  const glm::ivec2 below = { 0, 10 };

  // not quite 45 degrees each way
  const glm::ivec2 on_left_upper = { -10, -9 };
  const glm::ivec2 on_left_lower = { -10, 9 };
  const glm::ivec2 on_right_upper = { 10, -9 };
  const glm::ivec2 on_right_lower = { 10, 9 };
  const glm::ivec2 on_below_upper = { 9, 10 };
  const glm::ivec2 on_below_lower = { -9, 10 };
  const glm::ivec2 on_above_upper = { 9, -10 };
  const glm::ivec2 on_above_lower = { -9, -10 };

  // different backstab cases
  //
  std::cout << "Testing patrol case: looking_right" << std::endl;
  test_patrol(looking_right, on_left, true);
  test_patrol(looking_right, on_left_upper, true);
  test_patrol(looking_right, on_left_lower, true);
  test_patrol(looking_right, on_right, false);
  test_patrol(looking_right, above, false);
  test_patrol(looking_right, below, false);

  std::cout << "Testing patrol case: looking_left" << std::endl;
  test_patrol(looking_left, on_right, true);
  test_patrol(looking_left, on_right_upper, true);
  test_patrol(looking_left, on_right_lower, true);
  test_patrol(looking_left, on_left, false);
  test_patrol(looking_left, above, false);
  test_patrol(looking_left, below, false);

  std::cout << "Testing patrol case: looking_up" << std::endl;
  test_patrol(looking_up, below, true);
  test_patrol(looking_up, on_below_upper, true);
  test_patrol(looking_up, on_below_lower, true);
  test_patrol(looking_up, on_right, false);
  test_patrol(looking_up, on_left, false);
  test_patrol(looking_up, above, false);

  std::cout << "Testing patrol case: looking_down" << std::endl;
  test_patrol(looking_down, above, true);
  test_patrol(looking_down, on_above_upper, true);
  test_patrol(looking_down, on_above_lower, true);
  test_patrol(looking_down, on_right, false);
  test_patrol(looking_down, on_left, false);
  test_patrol(looking_down, below, false);
};

} // namespace tests

} // namespace game2d