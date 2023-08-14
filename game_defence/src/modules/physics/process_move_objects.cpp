// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "renderer/components.hpp"

// c++ lib headers
#include <algorithm>
#include <set>
#include <vector>

void
game2d::update_move_objects_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // update the physics view of the world
  auto physics_objects_view = r.view<PhysicsTransformComponent, const TransformComponent>();
  physics_objects_view.each([](auto entity, PhysicsTransformComponent& ptc, const TransformComponent& tc) {
    ptc.w = glm::abs(tc.scale.x);
    ptc.h = glm::abs(tc.scale.y);
    ptc.x_tl = static_cast<int>(tc.position.x - (ptc.w / 2.0f));
    ptc.y_tl = static_cast<int>(tc.position.y - (ptc.h / 2.0f));
    ptc.ent_id = static_cast<uint32_t>(entity);
  });

  // generates actor-solid collisions
  std::vector<Collision2D> collisions;

  // move grid actors,
  // stop if collides with an entity with the blocking component
  const auto& grid_actors = r.view<TransformComponent, GridMoveComponent, PhysicsTransformComponent>();
  for (auto [entity, transform, grid, ptc] : grid_actors.each()) {
    transform.position_dxdy.x += grid.x;
    transform.position_dxdy.y += grid.y;
    grid.x = 0;
    grid.y = 0;

    int move_x = static_cast<int>(transform.position_dxdy.x);
    const auto colls_x = do_move(r, entity, move_x, transform, ptc, CollisionAxis::x);
    collisions.insert(collisions.end(), colls_x.begin(), colls_x.end());

    int move_y = static_cast<int>(transform.position_dxdy.y);
    const auto colls_y = do_move(r, entity, move_y, transform, ptc, CollisionAxis::y);
    collisions.insert(collisions.end(), colls_y.begin(), colls_y.end());
  }

  // move velocity actors,
  // stop if collides with an entity with the blocking component
  const float seconds_dt = milliseconds_dt / 1000.0f;
  const auto& vel_actors = r.view<TransformComponent, const VelocityComponent, PhysicsTransformComponent>();
  for (auto [entity, transform, velocity, ptc] : vel_actors.each()) {
    transform.position_dxdy.x += velocity.x * (seconds_dt);
    transform.position_dxdy.y += velocity.y * (seconds_dt);

    int move_x = static_cast<int>(transform.position_dxdy.x);
    const auto colls_x = do_move(r, entity, move_x, transform, ptc, CollisionAxis::x);
    collisions.insert(collisions.end(), colls_x.begin(), colls_x.end());

    int move_y = static_cast<int>(transform.position_dxdy.y);
    const auto colls_y = do_move(r, entity, move_y, transform, ptc, CollisionAxis::y);
    collisions.insert(collisions.end(), colls_y.begin(), colls_y.end());
  }

  // add collision info
  for (const auto& coll : collisions) {
    uint32_t old_ent_id = coll.ent_id_0;
    uint32_t new_ent_id = coll.ent_id_1;
    uint64_t unique_collision_id = engine::encode_cantor_pairing_function(old_ent_id, new_ent_id);
    physics.frame_collisions[unique_collision_id] = coll;
  }

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
};