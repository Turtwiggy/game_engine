// header
#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/physics_box2d/components.hpp"
#include "renderer/components.hpp"

void
game2d::update_lifecycle_system(entt::registry& r, b2World& world, const uint64_t& milliseconds_dt)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>();
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);
    lifecycle.milliseconds_alive += static_cast<int>(milliseconds_dt);
  });

  // process destroyed objects
  for (const auto& entity : dead.dead) {
    auto* act_ptr = r.try_get<ActorComponent>(entity);
    if (act_ptr)
      world.DestroyBody(act_ptr->body);
    r.destroy(entity);
  }

  dead.dead.clear();

  //
  // process create requests
  //
  dead.created_this_frame.clear();

  const auto requests = r.view<CreateEntityRequest>();
  for (auto [entity, request] : requests.each()) {
    auto e = create_gameplay(r, world, request.type);

    auto* act_ptr = r.try_get<ActorComponent>(e);
    if (act_ptr) {
      // set position by physics
      b2Vec2 pos{ static_cast<float>(request.position.x), static_cast<float>(request.position.y) };
      act_ptr->body->SetTransform(pos, 0.0f);

      // set linear velocity
      b2Vec2 vel{ static_cast<float>(request.velocity.x), static_cast<float>(request.velocity.y) };
      act_ptr->body->SetLinearVelocity(vel);

    } else {
      // set position by transform
      auto& transform = r.get<TransformComponent>(e);
      transform.position = request.position;
    }

    // capture new entity
    dead.created_this_frame.push_back(e);

    // destroy request
    r.destroy(entity);
  }
};