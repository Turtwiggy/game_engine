#include "system.hpp"

#include "components.hpp"

#include "helpers/line.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_actor_legs_system(entt::registry& r, const float dt)
{
  static float time = 0.0f;
  time += dt;

  const auto& view = r.view<LegsComponent>();
  for (const auto& [e, legs] : view.each()) {
    const auto& [body_position, body_velocity] = r.get<const AABB, const VelocityComponent>(legs.body);

    // work out speed
    const bool walking = glm::abs(body_velocity.x) > 0.0f;
    const float speed = 5.0f;
    float active_speed = 0;
    if (walking && body_velocity.x > 0.0f)
      active_speed = speed;
    else if (walking && body_velocity.x < 0.0f)
      active_speed = -speed;

    float gait = 8;
    const float x = gait * cos(active_speed * time);
    const float y = gait * sin(active_speed * time);

    glm::ivec2 hip = body_position.center;
    hip.x += 5; // right leg
    hip.y += 16;

    glm::ivec2 knee = hip;
    knee.x += 6 + (x / 2.0f);
    knee.y += 6 + (y / 2.0f);

    glm::ivec2 foot = hip;
    foot.x += x;
    foot.y += 12 + y;

    set_transform_with_line(r, legs.lines[0], generate_line(hip, knee, 2));
    set_transform_with_line(r, legs.lines[1], generate_line(knee, foot, 2));
  }
}

} // namespace game2d