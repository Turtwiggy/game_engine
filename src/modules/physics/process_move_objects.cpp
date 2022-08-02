// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// c++ lib headers
#include <algorithm>
#include <iostream>
#include <vector>

void
game2d::update_move_objects_system(entt::registry& registry, uint64_t milliseconds_dt)
{
  // actors and solids never overlap,
  // and solids dont overlap with solids

  const auto& solids = registry.view<const VelocityComponent,
                                     const TransformComponent,
                                     const PhysicsSizeComponent,
                                     const PhysicsSolidComponent>();

  const auto& actors =
    registry
      .view<const VelocityComponent, TransformComponent, const PhysicsSizeComponent, const PhysicsActorComponent>();

  // move actors, but stop at solids
  actors.each([&solids, &milliseconds_dt](const auto& vel, auto& transform, const auto size, const auto& actor) {
    transform.position_dxdy.x += vel.x * (milliseconds_dt / 1000.0f);
    transform.position_dxdy.y += vel.y * (milliseconds_dt / 1000.0f);

    constexpr auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

    // x-axis
    int move_x = static_cast<int>(transform.position_dxdy.x);
    if (move_x != 0) {
      transform.position_dxdy.x -= move_x;
      int sign = Sign(move_x);
      while (move_x != 0) {
        //     aabb.x_tl = static_cast<int>(pos.x - (size.w / 2.0f));
        //     aabb.y_tl = static_cast<int>(pos.y - (size.h / 2.0f));
        // if(!collides(solids, pos.x + new vector2(sign, 0)))
        {
          transform.position.x += sign;
          move_x -= sign;
        }
        // else // hit a solid
        {
          // callback(registry)
        }
      }
    }

    // y-axis
    int move_y = static_cast<int>(transform.position_dxdy.y);
    if (move_y != 0) {
      transform.position_dxdy.y -= move_y;
      int sign = Sign(move_y);
      while (move_y != 0) {
        // while no solids collision...
        {
          transform.position.y += sign;
          move_y -= sign;
        }
        // else // hit a solid
        {
          // callback(registry)
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