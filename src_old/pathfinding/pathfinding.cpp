// your header
#include "pathfinding.hpp"

// components
#include "game/components/pathfinding.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>

void
game2d::update_pathfinding_system(entt::registry& registry)
{
  const auto& view = registry.view<const TransformComponent, DestinationComponent, VelocityComponent>();
  view.each([&registry](const auto& transform, auto& destination, auto& vel) {
    //
    // Stop object if at or past destination
    // bool at_x = destination.xy.x == transform.position.x;
    // bool at_y = destination.xy.y == transform.position.y;
    // bool past_x =
    //   glm::sign(vel.x) > 0 ? transform.position.x > destination.xy.x : transform.position.x < destination.xy.x;
    // bool past_y =
    //   glm::sign(vel.y) > 0 ? transform.position.y > destination.xy.y : transform.position.y < destination.xy.y;
    // if (at_x || past_x)
    //   vel.x = 0.0f;
    // if (at_y || past_y)
    //   vel.y = 0.0f;

    // // clear up destination line
    // if ((at_x || past_x) && (at_y || past_y) && destination.destination_line != entt::null) {
    //   registry.destroy(destination.destination_line);
    //   destination.destination_line = entt::null;
    // }
  });
};
