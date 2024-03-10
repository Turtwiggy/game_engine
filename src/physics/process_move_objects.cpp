// your header
#include "physics/process_move_objects.hpp"

// components
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"

#include "modules/actor_player/components.hpp" // shouldnt be here
#include "modules/lifecycle/components.hpp"    // shouldnt be here

#include "optick.h"

// c++ lib headers
#include <algorithm>
#include <set>
#include <vector>

namespace game2d {

void
update_aabb_based_on_rotation(entt::registry& r)
{
  // update all aabb size based on transform size
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
}

}

void
game2d::update_move_objects_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  update_aabb_based_on_rotation(r);

  // generates actor-solid collisions. note: touching, not inside each other (illegal)
  // std::set<Collision2D> actor_solid_collisions;

  // move velocity actors,
  // stop if collides with an entity with the blocking component
  {
    OPTICK_EVENT("Move Actors");

    const auto& vel_actors = r.view<AABB, VelocityComponent>(entt::exclude<WaitForInitComponent>);
    const auto& solids = r.view<const PhysicsSolidComponent, const AABB>(entt::exclude<WaitForInitComponent>);
    const float dt = milliseconds_dt / 1000.0f;

    for (const auto& [entity, aabb, vel] : vel_actors.each()) {

      // debug player
      if (const auto* p = r.try_get<PlayerComponent>(entity)) {
        int k = 1;
      }

      // move_x
      //
      {
        vel.remainder_x += vel.x * dt;

#if defined(_DEBUG)
        if (isnan(vel.remainder_x))
          std::cout << "warning: actor has NaN remainder_x" << std::endl;
#endif

        int move = static_cast<int>(vel.remainder_x);

        if (move != 0) {
          vel.remainder_x -= move; // consume so no frame jump

          const int sign = glm::sign(move);

          for (int i = 0; i < glm::abs(move); i++) {
            bool hit_solid = false;

            // would-be updated position
            AABB updated_pos = aabb;
            updated_pos.center.x += sign;

            // Check if the updated position would collide with anything
            for (const auto& [o_entity, o_psolid, o_aabb] : solids.each()) {
              const bool same = entity == o_entity;
              if (!same && collide(updated_pos, o_aabb)) {
                // Collision2D collision;
                // const auto id_0 = static_cast<uint32_t>(entity);
                // const auto id_1 = static_cast<uint32_t>(o_entity);
                // collision.ent_id_0 = glm::min(id_0, id_1);
                // collision.ent_id_1 = glm::max(id_0, id_1);
                // actor_solid_collisions.emplace(collision);

                move = 0;
                hit_solid = true;
                break; // a collision
              }
            }

            if (hit_solid)
              break;

            // If no collision...
            aabb.center.x += sign;
            move -= sign;
          }
        }
      } // end move x
      //
      // move_y
      //
      {
        vel.remainder_y += vel.y * dt;
        int amount = static_cast<int>(vel.remainder_y);
        if (amount != 0) {
          vel.remainder_y -= amount; // consume so no frame jump
          const int sign = glm::sign(amount);

          for (int i = 0; i < glm::abs(amount); i++) {
            bool hit_solid = false;

            // would-be updated position
            AABB updated_pos = aabb;
            updated_pos.center.y += sign;

            // Check if the updated position would collide with anything
            for (const auto& [o_entity, o_psolid, o_aabb] : solids.each()) {
              const bool same = entity == o_entity;
              if (!same && collide(updated_pos, o_aabb)) {

                // Collision2D collision;
                // const auto id_0 = static_cast<uint32_t>(entity);
                // const auto id_1 = static_cast<uint32_t>(o_entity);
                // collision.ent_id_0 = glm::min(id_0, id_1);
                // collision.ent_id_1 = glm::max(id_0, id_1);
                // actor_solid_collisions.emplace(collision);

                amount = 0;
                hit_solid = true;
                break; // a collision
              }
            }

            if (hit_solid)
              break;

            // Move player if empty space
            aabb.center.y += sign;
            amount -= sign;
          }
        }
      } // end move y
    }
  }

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying

  // check if any actor-solids are overlapping... this is illegal

  // Update PhysicsTransform View (for sweep-and-prune view)
  //
  const auto& physics_transform_view = r.view<const AABB, PhysicsTransformXComponent, PhysicsTransformYComponent>();
  for (const auto& [entity, aabb, px, py] : physics_transform_view.each()) {
    px.l = aabb.center.x - (glm::abs(aabb.size.x) / 2);
    px.r = aabb.center.x + (glm::abs(aabb.size.x) / 2);
    py.b = aabb.center.y - (glm::abs(aabb.size.y) / 2);
    py.t = aabb.center.y + (glm::abs(aabb.size.y) / 2);
  };

  // Update all transforms
  //
  const auto& physics_objects_view = r.view<const AABB, TransformComponent>(entt::exclude<SeperateTransformFromAABB>);
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

  // add actor-solid collisions
  // auto& p = get_first_component<SINGLETON_PhysicsComponent>(r);
  // p.frame_solid_collisions = actor_solid_collisions;
};