#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "glm/glm.hpp"
#include "maths/maths.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_enemy_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const auto& first_player = get_first<PlayerComponent>(r);
  if (first_player == entt::null)
    return;
  const auto& first_player_transform = r.get<const TransformComponent>(first_player);

  const auto& view = r.view<const TransformComponent, const EnemyComponent>();
  for (auto [entity, transform, enemy] : view.each()) {
    // move towards player
    glm::ivec3 d = first_player_transform.position - transform.position;
    if (d.x == 0 && d.y == 0 && d.z == 0) // check same spot not clicked
      continue;

    glm::vec2 dir = glm::vec2(d.x, d.y);
    glm::vec2 n = dir;

    if (dir.x != 0.0f || dir.y != 0.0f)
      n = normalize(dir);

    const float enemy_speed = 120.0f;
    // grid.x += n.x * enemy_speed * (milliseconds_dt / 1000.0f);
    // grid.y += n.y * enemy_speed * (milliseconds_dt / 1000.0f);
  }
}

} // namespace game2d