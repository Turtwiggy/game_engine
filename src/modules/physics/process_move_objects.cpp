// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// c++ lib headers
#include <algorithm>
#include <vector>

void
game2d::update_move_objects_system(entt::registry& registry, uint64_t milliseconds_dt)
{
  // actors and solids never overlap,
  // and solids dont overlap with solids
  constexpr auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

  std::vector<PhysicsObject> solids;
  get_solids_as_physics_objects(registry, solids);

  const auto& actors =
    registry
      .view<TransformComponent, const VelocityComponent, const PhysicsSizeComponent, const PhysicsActorComponent>();

  // move actors, but stop at solids
  float seconds_dt = milliseconds_dt / 1000.0f;
  actors.each([&solids, &seconds_dt](TransformComponent& transform,
                                     const VelocityComponent& vel,
                                     const PhysicsSizeComponent& size,
                                     const PhysicsActorComponent& actor) {
    transform.position_dxdy.x += vel.x * seconds_dt;
    transform.position_dxdy.y += vel.y * seconds_dt;

    // x-axis
    int move_x = static_cast<int>(transform.position_dxdy.x);
    if (move_x != 0) {
      transform.position_dxdy.x -= move_x;
      int sign = Sign(move_x);
      while (move_x != 0) {

        // updated position
        PhysicsObject po;
        po.w = transform.scale.x;
        po.h = transform.scale.y;
        po.x_tl = (transform.position.x - (transform.scale.x / 2.0f)) + sign;
        po.y_tl = (transform.position.y - (transform.scale.y / 2.0f));
        po.collidable = true;

        if (!collides(po, solids)) {
          transform.position.x += sign;
          move_x -= sign;
        } else {
          // std::cout << "hit a solid";
          // callback(registry)
          break;
        }
      }
    }

    // y-axis
    int move_y = static_cast<int>(transform.position_dxdy.y);
    if (move_y != 0) {
      transform.position_dxdy.y -= move_y;
      int sign = Sign(move_y);
      while (move_y != 0) {

        // updated position
        PhysicsObject po;
        po.w = transform.scale.x;
        po.h = transform.scale.y;
        po.x_tl = (transform.position.x - (transform.scale.x / 2.0f));
        po.y_tl = (transform.position.y - (transform.scale.y / 2.0f)) + sign;
        po.collidable = true;

        if (!collides(po, solids)) {
          transform.position.y += sign;
          move_y -= sign;
        } else {
          // std::cout << "hit a solid";
          // callback(registry)
          break;
        }
      }
    }
  });

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
};