#include "resolve_collisions.hpp"

#include "game/components/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"

#include <format>

void
game2d::update_resolve_collisions_system(Game& game)
{
  auto& eb = game.dead;
  auto& r = game.state;

  { // take damage on collision
    const auto& view = r.view<WasCollidedWithComponent, HealthComponent>();
    view.each([&game, &eb](auto entity, WasCollidedWithComponent& coll, HealthComponent& hp) {
      //
      hp.hp -= 1;
      game.ui_events.events.push_back(std::format("someone took {} damage, new_hp: {}", 1, hp.hp));

      if (hp.hp <= 0) {
        eb.dead.emplace(entity);
        game.ui_events.events.push_back("somoene died :O");
      }
    });
  }

  { // flash sprite on collision
    const auto& view = r.view<WasCollidedWithComponent, HealthComponent>();
    // TODO: this
  }

  const auto& view = r.view<WasCollidedWithComponent>();
  r.remove<WasCollidedWithComponent>(view.begin(), view.end());
};