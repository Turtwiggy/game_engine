#include "pch.hpp"

#include "engine/physics/physics_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/collisions/resolve_collisions_helpers.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

void
convert_box2d_coll_to_entt(entt::registry& r, const b2ShapeId a, const b2ShapeId b, auto callback)
{
  const auto fixture_eid_a = static_cast<entt::entity>((reinterpret_cast<uintptr_t>(b2Shape_GetUserData(a))));
  const auto fixture_eid_b = static_cast<entt::entity>((reinterpret_cast<uintptr_t>(b2Shape_GetUserData(b))));
  callback(fixture_eid_a, fixture_eid_b);
};

void
update_physics_system(entt::registry& r, const uint64_t ms_dt)
{
  const auto physics_e = get_first<SINGLE_Physics>(r);
  if (physics_e == entt::null)
    return;
  const auto& physics = get_first_component<SINGLE_Physics>(r);

  // update world
  {
    const auto dt = ms_dt / 1000.0f;
    const int substep_count = 4;
    b2World_Step(physics.worldId, dt, substep_count);
  }

  // Generate contact events.
  {
    const b2ContactEvents c_events = b2World_GetContactEvents(physics.worldId);
    const b2SensorEvents s_events = b2World_GetSensorEvents(physics.worldId);

    for (int i = 0; i < c_events.beginCount; ++i) {
      b2ContactBeginTouchEvent* beginEvent = c_events.beginEvents + i;
      convert_box2d_coll_to_entt(r, beginEvent->shapeIdA, beginEvent->shapeIdB, [&r](const auto e_a, const auto e_b) {
        SINGLE_Events::instance.dispatcher->trigger(OnCollisionEnter{ .a = e_a, .b = e_b });
      });
    }
    for (int i = 0; i < c_events.endCount; ++i) {
      b2ContactEndTouchEvent* endEvent = c_events.endEvents + i;
      if (b2Shape_IsValid(endEvent->shapeIdA) && b2Shape_IsValid(endEvent->shapeIdB)) {
        convert_box2d_coll_to_entt(r, endEvent->shapeIdA, endEvent->shapeIdB, [](const auto e_a, const auto e_b) {
          SINGLE_Events::instance.dispatcher->trigger(OnCollisionExit{ .a = e_a, .b = e_b });
        });
      }
    }
    for (int i = 0; i < s_events.beginCount; ++i) {
      b2SensorBeginTouchEvent* beginEvent = s_events.beginEvents + i;
      convert_box2d_coll_to_entt(
        r, beginEvent->sensorShapeId, beginEvent->visitorShapeId, [&r](const auto e_a, const auto e_b) {
          SINGLE_Events::instance.dispatcher->trigger(OnCollisionEnter{ .a = e_a, .b = e_b });
        });
    }
    for (int i = 0; i < s_events.endCount; ++i) {
      b2SensorEndTouchEvent* endEvent = s_events.endEvents + i;
      if (b2Shape_IsValid(endEvent->sensorShapeId) && b2Shape_IsValid(endEvent->visitorShapeId)) {
        convert_box2d_coll_to_entt(r, endEvent->sensorShapeId, endEvent->visitorShapeId, [](const auto e_a, const auto e_b) {
          SINGLE_Events::instance.dispatcher->trigger(OnCollisionExit{ .a = e_a, .b = e_b });
        });
      }
    }
    SINGLE_Events::instance.dispatcher->update();
  }

  // update renderer
  {
    // const auto& view = r.view<const PhysicsBodyComponent, TransformComponent>(entt::exclude<SeparateTransformAndAABB>);
    const auto& view = r.view<const PhysicsBodyComponent, TransformComponent>();
    for (const auto& [e, body_c, transform_c] : view.each()) {

      const auto pos = meters_to_pixels(b2Body_GetPosition(body_c.bodyId));
      transform_c.position.x = pos.x;
      transform_c.position.y = pos.y;

      // don't update the sprite scale.
      // when the physics object rotates,
      // the aabb grows which makes the sprite look like it's "jumping"
      // const auto& size = get_size(r, e);
      // transform_c.scale.x = size.x;
      // transform_c.scale.y = size.y;
    }
  }

  {
    const auto view = r.view<const PhysicsFixtureComponent, TransformComponent, HasParentComponent>();
    for (const auto& [e, fixture_c, transform_c, parent_c] : view.each()) {
      const auto parent_e = parent_c.parent;
      if (parent_e == entt::null || !r.valid(parent_e)) {
        r.remove<HasParentComponent>(e);
        continue;
      }
      const auto parent_pos_pixels = get_position(r, parent_e);

      // const auto aabb = fixture_c.fixture->GetAABB(0);
      // const auto offset = meters_to_pixels(aabb.GetCenter() - fixture_c.body->GetWorldCenter());
      // transform_c.position.x = parent_pos_pixels.x + offset.x;
      // transform_c.position.y = parent_pos_pixels.y + offset.y;
      transform_c.position.x = parent_pos_pixels.x;
      transform_c.position.y = parent_pos_pixels.y;
    }
  }

  {
    const auto& view =
      r.view<const PhysicsBodyComponent, TransformComponent, const SetTransformRotationBasedOnPhysicsBody>();
    for (const auto& [e, body_c, transform_c, req_c] : view.each()) {
      const auto angle = b2Rot_GetAngle(b2Body_GetRotation(body_c.bodyId));
      transform_c.rotation_radians.z = angle;
    }
  }

  {
    const auto& view =
      r.view<const PhysicsBodyComponent, TransformComponent, const SetTransformRotationBasedOnPhysicsVelocity>();
    for (const auto& [e, body_c, transform_c, req_c] : view.each()) {
      const auto vel = b2Body_GetLinearVelocity(body_c.bodyId);
      const auto dir = engine::normalize_safe({ vel.x, vel.y });
      const auto angle = engine::dir_to_angle_radians({ dir.x, dir.y });
      transform_c.rotation_radians.z = angle;
    }
  }
}

} // namespace game2d