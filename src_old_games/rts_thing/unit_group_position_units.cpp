#include "unit_group_position_units.hpp"

#include "game/components/pathfinding.hpp"
#include "game/components/units.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

#include "engine/maths/maths.hpp"

namespace game2d {

void
update_unit_group_position_units_system(entt::registry& registry)
{
  const auto& groups = registry.view<const UnitGroupComponent, TransformComponent, const PhysicsSizeComponent>();
  groups.each([&registry](const auto& group, auto& transform, const auto& physics_size) {
    int group_units = group.units.size();
    if (group_units == 0)
      return;
    float& group_angle = transform.rotation.z;

    // TEMP: slowly rotate
    // group_angle += engine::PI / 8.0f * dt;

    int half_units = group_units / 2;
    int unit_index = -half_units; // go from -half_units to half_units

    int larger_side = physics_size.w > physics_size.h ? physics_size.w : physics_size.h;

    int unit_offset = 0;
    if (half_units != 0)
      unit_offset = (larger_side / 2) / half_units;

    for (const auto& unit : group.units) {
      auto& unittransform = registry.get<TransformComponent>(unit);
      auto& unitpath = registry.get<DestinationComponent>(unit);
      auto& unitvel = registry.get<VelocityComponent>(unit);

      glm::ivec2 unitdir = engine::angle_radians_to_direction(group_angle);
      glm::ivec2 destination = transform.position - unittransform.position;
      destination += (unitdir * unit_index * unit_offset);
      unit_index += 1;

      // check same spot not clicked
      glm::ivec2 d = { destination.x, destination.y };
      if (d.x == 0 && d.y == 0)
        continue;

      // dont go back and forth on a pixel
      if (glm::abs(d.x) <= 1 && glm::abs(d.y) <= 1)
        continue;

      // set velocity to get to destination
      glm::vec2 dir = { d.x, d.y };
      glm::vec2 n = normalize(dir);
      const float speed = 100.0f;
      unitvel.x = n.x * speed;
      unitvel.y = n.y * speed;

      // set destination
      unitpath.xy = destination;
    }
  });
};

} // namespace game2d