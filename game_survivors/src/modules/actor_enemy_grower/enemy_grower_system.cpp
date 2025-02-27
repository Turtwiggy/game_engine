#include "enemy_grower_system.hpp"

#include "enemy_grower_components.hpp"
#include "enemy_grower_helpers.hpp"

#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat_scale_on_hit/components.hpp"

namespace game2d {

void
update_enemy_grower_system(entt::registry& r, float dt)
{
  static float speed = 5.0f;

  const auto view = r.view<GrowerComponent, PhysicsBodyComponent, TransformComponent>();
  for (const auto& [body_e, grower_c, body_c, t_c] : view.each()) {
    grower_c.radius_pixels += speed * dt;
    grower_c.radius_pixels = glm::clamp(grower_c.radius_pixels, 0.0f, 256.0f);

    for (const entt::entity& fix_e : body_c.fixtures) {
      update_circle_fixture_size(r, body_e, fix_e, grower_c.radius_pixels);

      float rad_pixels = grower_c.radius_pixels;
      const auto size = glm::vec2{ 2.0f * rad_pixels, 2.0f * rad_pixels };
      t_c.scale.x = size.x;
      t_c.scale.y = size.y;
      r.get<DefaultSizeComponent>(body_e).size = size;
    }
  }
}

} // namespace game2d