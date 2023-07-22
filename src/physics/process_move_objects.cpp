// your header
#include "physics/process_move_objects.hpp"

// components
#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/components.hpp"

// c++ lib headers
#include <algorithm>
#include <set>
#include <vector>

void
game2d::update_move_objects_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  // update the physics view of the world
  auto physics_objects_view = r.view<PhysicsTransformComponent, const TransformComponent>();
  physics_objects_view.each([](auto entity, PhysicsTransformComponent& ptc, const TransformComponent& tc) {
    ptc.w = glm::abs(tc.scale.x);
    ptc.h = glm::abs(tc.scale.y);
    ptc.x_tl = static_cast<int>(tc.position.x - (ptc.w / 2.0f));
    ptc.y_tl = static_cast<int>(tc.position.y - (ptc.h / 2.0f));
    ptc.ent_id = static_cast<uint32_t>(entity);
  });

  // move grid actors,
  // stop if collides with an entity with the blocking component
  const auto& grid_actors = r.view<TransformComponent, GridMoveComponent, PhysicsTransformComponent>();
  for (auto [entity, transform, move, ptc] : grid_actors.each()) {
    transform.position_dxdy.x += move.x;
    transform.position_dxdy.y += move.y;
    move.x = 0;
    move.y = 0;

    int move_x = static_cast<int>(transform.position_dxdy.x);
    const auto coll_x = do_move(r, entity, move_x, transform, ptc, CollisionAxis::x);

    int move_y = static_cast<int>(transform.position_dxdy.y);
    const auto coll_y = do_move(r, entity, move_y, transform, ptc, CollisionAxis::y);

    // LIMITATION: this only emplaces one component, and an entity could be collided with multiple times.
    if (coll_x) {
      const auto instigator = static_cast<entt::entity>(coll_x->ent_id_0);
      const auto other = static_cast<entt::entity>(coll_x->ent_id_1);
      r.emplace_or_replace<WasCollidedWithComponent>(other, instigator);
    }
    // LIMITATION: this only emplaces one component, and an entity could be collided with multiple times.
    if (coll_y) {
      const auto instigator = static_cast<entt::entity>(coll_y->ent_id_0);
      const auto other = static_cast<entt::entity>(coll_y->ent_id_1);
      r.emplace_or_replace<WasCollidedWithComponent>(other, instigator);
    }
  }

  // move velocity actors,
  // stop if collides with an entity with the blocking component
  const float seconds_dt = milliseconds_dt / 1000.0f;
  const auto& vel_actors = r.view<TransformComponent, const VelocityComponent, PhysicsTransformComponent>();
  for (auto [entity, transform, velocity, ptc] : vel_actors.each()) {
    transform.position_dxdy.x += velocity.x * (seconds_dt);
    transform.position_dxdy.y += velocity.y * (seconds_dt);

    int move_x = static_cast<int>(transform.position_dxdy.x);
    const auto coll_x = do_move(r, entity, move_x, transform, ptc, CollisionAxis::x);
    ptc.x_tl = static_cast<int>(transform.position.x - (ptc.w / 2.0f));

    int move_y = static_cast<int>(transform.position_dxdy.y);
    const auto coll_y = do_move(r, entity, move_y, transform, ptc, CollisionAxis::y);
    ptc.y_tl = static_cast<int>(transform.position.y - (ptc.h / 2.0f));
  }

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
};