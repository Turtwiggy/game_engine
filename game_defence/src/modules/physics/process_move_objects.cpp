// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "actors.hpp"
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
  // auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  //
  // update all aabb based on rotation
  //
  const auto& physics_rot_objects = r.view<const TransformComponent, AABB>();
  for (const auto& [entity, t, aabb] : physics_rot_objects.each()) {
    const float& theta = t.rotation_radians.z;

    const float epsilon = 0.001f;
    if (glm::abs(theta) <= epsilon && glm::abs(theta) >= -epsilon) {
      aabb.size = { t.scale.x, t.scale.y };
      continue;
    }
    // https://stackoverflow.com/questions/6657479/aabb-of-rotated-sprite
    const float w0 = t.scale.y * std::sin(theta);
    const float w1 = t.scale.x * std::cos(theta);
    const float h0 = t.scale.x * std::sin(theta);
    const float h1 = t.scale.y * std::cos(theta);
    const float rotated_w = glm::abs(w0) + glm::abs(w1);
    const float rotated_h = glm::abs(h0) + glm::abs(h1);
    aabb.size = { rotated_w, rotated_h };
  }

  // generates actor-solid collisions
  // std::vector<Collision2D> collisions;

  // move velocity actors,
  // stop if collides with an entity with the blocking component
  const auto& vel_actors = r.view<AABB, VelocityComponent>();
  for (const auto& [entity, aabb, vel] : vel_actors.each()) {

    const auto et = r.get<EntityTypeComponent>(entity);

    const auto colls_x = do_move_x(r, entity, aabb, vel);
    // collisions.insert(collisions.end(), colls_x.begin(), colls_x.end());

    const auto colls_y = do_move_y(r, entity, aabb, vel);
    // collisions.insert(collisions.end(), colls_x.begin(), colls_x.end());
  }

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying

  //
  // Update PhysicsTransform View (for sweep-and-prune view)
  //
  const auto& physics_transform_view = r.view<const AABB, PhysicsTransformXComponent, PhysicsTransformYComponent>();
  for (const auto& [entity, aabb, px, py] : physics_transform_view.each()) {
    px.l = aabb.center.x - (glm::abs(aabb.size.x) / 2);
    px.r = aabb.center.x + (glm::abs(aabb.size.x) / 2);
    py.b = aabb.center.y - (glm::abs(aabb.size.y) / 2);
    py.t = aabb.center.y + (glm::abs(aabb.size.y) / 2);
  };

  //
  // Update all transforms
  //
  const auto& physics_objects_view = r.view<const AABB, TransformComponent>();
  for (const auto& [entity, aabb, t] : physics_objects_view.each()) {
    t.position.x = aabb.center.x;
    t.position.y = aabb.center.y;
  }

  // debug aabbs
  // const auto& aabbs = r.view<const AABB>();
  // for (const auto& [entity, aabb] : aabbs.each()) {
  //   auto& transform = r.get<TransformComponent>(aabb.debug_aabb);
  //   transform.position = { aabb.center.x, aabb.center.y, 0 };
  //   transform.scale = { aabb.size.x, aabb.size.y, 0 };
  // }
};