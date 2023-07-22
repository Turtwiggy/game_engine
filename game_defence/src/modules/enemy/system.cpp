#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "glm/glm.hpp"
#include "maths/maths.hpp"
#include "modules/player/components.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

namespace game2d {

void
update_enemy_system(entt::registry& r, const uint64_t& ms_dt)
{
  const auto& first_player = get_first<PlayerComponent>(r);
  const auto& first_player_transform = r.get<const TransformComponent>(first_player);

  const auto& view = r.view<GridMoveComponent, const TransformComponent, const EnemyComponent>();
  for (auto [entity, grid, transform, enemy] : view.each()) {
    // move towards player
    glm::ivec3 d = first_player_transform.position - transform.position;
    if (d.x == 0 && d.y == 0 && d.z == 0) // check same spot not clicked
      continue;

    glm::vec2 dir = glm::vec2(d.x, d.y);
    glm::vec2 n = dir;
    if (dir.x != 0.0f && dir.y != 0.0f)
      n = normalize(dir);

    const float speed = 2.0f;

    grid.x += static_cast<int>(n.x * speed);
    grid.y += static_cast<int>(n.y * speed);
  }
}

} // namespace game2d