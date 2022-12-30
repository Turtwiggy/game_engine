#include "system.hpp"

#include "components/events.hpp"
#include "helpers.hpp"

#include <entt/entt.hpp>

void
game2d::update_resolve_collisions_system(Game& game)
{
  check_if_damageable_received_collision(game);
  check_if_enemy_collided_with_cursor(game);
  check_if_player_collided_with_pickup(game);
  check_if_player_collided_with_exit(game);

  // clear all WasCollidedWithComponents
  auto& r = game.state;
  const auto& view = r.view<WasCollidedWithComponent>();
  r.remove<WasCollidedWithComponent>(view.begin(), view.end());
};