// header
#include "system.hpp"

#include "actors.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

void
game2d::update_lifecycle_system(SINGLETON_EntityBinComponent& dead, entt::registry& r, const uint64_t& milliseconds_dt)
{
  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>();
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);
    lifecycle.milliseconds_alive += milliseconds_dt;
  });

  // process destroyed objects
  for (const auto& entity : dead.dead)
    r.destroy(entity);

  dead.dead.clear();

  // process create requests
  const auto requests = r.view<CreateEntityRequest>();
  for (auto [entity, request] : requests.each()) {
    auto e = create_gameplay(r, request.entity_type);

    // set position
    auto& new_transform = r.get<TransformComponent>(e);
    new_transform.position = request.position;

    // set velocity
    auto* vel_ptr = r.try_get<VelocityComponent>(e);
    if (vel_ptr) {
      vel_ptr->x = request.velocity.x;
      vel_ptr->y = request.velocity.y;
    }

    r.destroy(entity);
  }
};