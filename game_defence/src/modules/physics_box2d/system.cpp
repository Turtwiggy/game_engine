#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_physics_box2d_system(entt::registry& r, b2World& world, const uint64_t& milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  { // update world
    const int vel_iterations = 8;
    const int pos_iterations = 3;
    world.Step(dt, vel_iterations, pos_iterations);
  }

  { // update renderer
    const auto& view = r.view<ActorComponent, TransformComponent>();
    for (auto [entity, actor, transform] : view.each()) {
      const b2Vec2& position = actor.body->GetPosition();
      const float& angle = actor.body->GetAngle();
      transform.position.x = static_cast<int>(position.x);
      transform.position.y = static_cast<int>(position.y);
    }
  }
}

} // namespace game2d
