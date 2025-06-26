#include "pch.hpp"

#include "enemy_grower_system.hpp"

#include "enemy_grower_components.hpp"
#include "enemy_grower_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"

namespace game2d {

void
update_enemy_grower_system(entt::registry& r, float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto view = r.view<const GrowerComponent, TransformComponent, DefaultSizeComponent>();
  for (const auto& [body_e, grower_c, t_c, default_size_c] : view.each()) {

    const auto fixture_e = get_fixture_by_tag(r, body_e, "fixture_core");

    auto& hp_c = r.get<HealthComponent>(fixture_e);

    // const float diameter_pixels = engine::scale(hp_c.hp, 0, hp_c.max_hp, 32, 256);
    const float diameter_pixels = 64; // fixed size. updates once.

    // update fixture
    update_circle_fixture_size(r, body_e, fixture_e, diameter_pixels);

    // update transform
    const auto size = glm::vec2{ diameter_pixels, diameter_pixels };
    t_c.scale.x = size.x;
    t_c.scale.y = size.y;
    default_size_c.size = size;
  }
}

} // namespace game2d