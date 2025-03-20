#include "pch.hpp"

#include "enemy_grower_system.hpp"

#include "enemy_grower_components.hpp"
#include "enemy_grower_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"

namespace game2d {

void
update_enemy_grower_system(entt::registry& r, float dt)
{
  static float speed = 5.0f;

  const auto view = r.view<GrowerComponent, TransformComponent>();
  for (const auto& [body_e, grower_c, t_c] : view.each()) {

    const auto fixture_e = get_fixture_by_tag(r, body_e, "fixture_core");

    auto& hp_c = r.get<HealthComponent>(fixture_e);

    const float radius_pixels = engine::scale(hp_c.hp, 0, hp_c.max_hp, 16, 256);

    // update fixture
    update_circle_fixture_size(r, body_e, fixture_e, radius_pixels);

    // update transform
    const auto size = glm::vec2{ 2.0f * radius_pixels, 2.0f * radius_pixels };
    t_c.scale.x = size.x;
    t_c.scale.y = size.y;
    r.get<DefaultSizeComponent>(body_e).size = size;
  }
}

} // namespace game2d