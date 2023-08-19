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

  // moves all physics transforms to where the visual transform is
  // this seems the wrong way round

  auto physics_objects_view = r.view<PhysicsTransformXComponent, PhysicsTransformYComponent, const TransformComponent>();
  for (const auto& [entity, x, y, t] : physics_objects_view.each()) {
    x.l = t.position.x - (glm::abs(t.scale.x) / 2.0f);
    x.r = t.position.x + (glm::abs(t.scale.x) / 2.0f);
    y.b = t.position.y - (glm::abs(t.scale.y) / 2.0f);
    y.t = t.position.y + (glm::abs(t.scale.y) / 2.0f);
  }

  // generates actor-solid collisions
  std::vector<Collision2D> collisions;

  // move grid actors,
  // stop if collides with an entity with the blocking component
  const auto& grid_actors =
    r.view<TransformComponent, GridMoveComponent, PhysicsTransformXComponent, PhysicsTransformYComponent>();
  for (auto [entity, transform, grid, x, y] : grid_actors.each()) {
    transform.position_dxdy.x += grid.x;
    transform.position_dxdy.y += grid.y;
    grid.x = 0;
    grid.y = 0;

    int move_x = static_cast<int>(transform.position_dxdy.x);
    const auto colls_x = do_move_x(r, entity, move_x, transform, x, y);
    collisions.insert(collisions.end(), colls_x.begin(), colls_x.end());

    int move_y = static_cast<int>(transform.position_dxdy.y);
    const auto colls_y = do_move_y(r, entity, move_y, transform, x, y);
    collisions.insert(collisions.end(), colls_y.begin(), colls_y.end());
  }

  // move velocity actors,
  // stop if collides with an entity with the blocking component
  const float seconds_dt = milliseconds_dt / 1000.0f;
  const auto& vel_actors =
    r.view<TransformComponent, const VelocityComponent, PhysicsTransformXComponent, PhysicsTransformYComponent>();
  for (auto [entity, transform, velocity, x, y] : vel_actors.each()) {
    transform.position_dxdy.x += velocity.x * (seconds_dt);
    transform.position_dxdy.y += velocity.y * (seconds_dt);

    int move_x = static_cast<int>(transform.position_dxdy.x);
    const auto colls_x = do_move_x(r, entity, move_x, transform, x, y);
    collisions.insert(collisions.end(), colls_x.begin(), colls_x.end());

    int move_y = static_cast<int>(transform.position_dxdy.y);
    const auto colls_y = do_move_y(r, entity, move_y, transform, x, y);
    collisions.insert(collisions.end(), colls_y.begin(), colls_y.end());
  }

  // add collision info
  for (const auto& coll : collisions)
    physics.frame_collisions.push_back(coll);

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
};