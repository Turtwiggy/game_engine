#include "resolve_collisions.hpp"

#include "game/components/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"

void
game2d::update_resolve_collisions_system(entt::registry& r)
{
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();

  { // take damage on collision
    const auto& view = r.view<WasCollidedWithComponent, HealthComponent>();
    view.each([&r, &eb](auto entity, WasCollidedWithComponent& coll, HealthComponent& hp) {
      //
      hp.hp -= 1;
      printf("entity hp: %i", hp.hp);

      if (hp.hp <= 0)
        eb.dead.emplace(entity);
    });
  }

  { // flash sprite on collision
    const auto& view = r.view<WasCollidedWithComponent, HealthComponent>();
    // TODO: this
  }

  const auto& view = r.view<WasCollidedWithComponent>();
  r.remove<WasCollidedWithComponent>(view.begin(), view.end());
};