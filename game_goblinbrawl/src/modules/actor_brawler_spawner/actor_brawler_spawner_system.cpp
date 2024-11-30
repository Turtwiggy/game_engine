#include "actor_brawler_spawner_system.hpp"

#include "actor_brawler_spawner_components.hpp"
#include "actors/actor_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/scene/scene_brawl_helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"

namespace game2d {

void
update_actor_brawler_spawner_system(entt::registry& r)
{
  const auto& view = r.view<const BrawlerSpawner, const TransformComponent, CooldownComponent>();

  for (const auto& [e, spawner_c, t_c, cooldown_c] : view.each()) {
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    // todo: check if the spawn zone is clear..?
    create_brawler(r, spawner_c, { t_c.position.x, t_c.position.y });
  }
}

} // namespace game2d