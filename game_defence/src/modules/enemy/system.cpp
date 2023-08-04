#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "glm/glm.hpp"
#include "maths/maths.hpp"
#include "modules/hearth/components.hpp"
#include "modules/physics_box2d/components.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_enemy_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  // SET TARGET
  // Note: this should be closest target
  const auto& first_target = get_first<HearthComponent>(r);
  if (first_target == entt::null)
    return;
  const auto& first_target_transform = r.get<const TransformComponent>(first_target);

  const auto& view = r.view<const TransformComponent, ActorComponent, const EnemyComponent>();
  for (auto [entity, transform, physics, enemy] : view.each()) {
    // move towards player
    glm::ivec3 d = first_target_transform.position - transform.position;
    if (d.x == 0 && d.y == 0 && d.z == 0) // check same spot not clicked
      continue;

    glm::vec2 dir = glm::vec2(d.x, d.y);

    glm::vec2 n = dir;
    if (dir.x != 0.0f || dir.y != 0.0f)
      n = normalize(dir);

    const float enemy_speed = 120.0f;
    b2Vec2 pdir{ n.x * enemy_speed, n.y * enemy_speed };
    physics.body->SetLinearVelocity(pdir);
  }
}

} // namespace game2d