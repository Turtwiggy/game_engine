#include "debug_fixtures_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"

namespace game2d {

void
update_debug_fixtures_system(entt::registry& r)
{
  const auto& physics_c = get_first_component<SINGLE_Physics>(r);

  for (b2Body* body = physics_c.world->GetBodyList(); body; body = body->GetNext()) {
    for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {

      // Retrieve fixture details
      const b2Shape* shape = fixture->GetShape();
      const auto fixture_e = (entt::entity)fixture->GetUserData().pointer;

      if (shape->GetType() == b2Shape::e_circle) {
        const auto* circle = static_cast<const b2CircleShape*>(shape);
        const float radius_m = circle->m_radius;
        const float radius = meters_to_pixels(radius_m);
        const auto pos_m = b2Vec2{ body->GetPosition().x, body->GetPosition().y };
        const auto pos = meters_to_pixels(pos_m);

        constexpr int segments = 32;
        constexpr float angle_step = engine::TWO_PI / segments;

        glm::vec2 prev_vert = pos + glm::vec2(radius, 0);

        for (int i = 1; i <= segments; i++) {
          float angle = i * angle_step;
          glm::vec2 cur_vert = pos + radius * glm::vec2{ cos(angle), sin(angle) };
          auto line = generate_line(cur_vert, prev_vert, 2.0f);

          Sprite s;
          s.sprite = "EMPTY";
          s.pos = line.position;
          s.size = line.scale;
          s.z_rotation = line.rotation;
          s.col.a = 0.5f * 255;
          s.z_idx = ZLayer::BACKGROUND;
          draw_sprite(r, s);

          prev_vert = cur_vert;
        }
      }

      else if (shape->GetType() == b2Shape::e_polygon) {

        const auto* polygon = static_cast<const b2PolygonShape*>(shape);

        const b2Transform transform = body->GetTransform();
        b2AABB aabb;
        polygon->ComputeAABB(&aabb, transform, 0);

        // Calculate size and position
        const auto size_m = aabb.upperBound - aabb.lowerBound;
        const auto center_m = 0.5f * (aabb.upperBound + aabb.lowerBound);

        // Create the sprite
        Sprite s;
        s.sprite = "EMPTY";
        s.pos = meters_to_pixels(center_m);
        s.size = meters_to_pixels(size_m);
        s.col.a = 0.5f * 255;
        s.z_idx = ZLayer::BACKGROUND;
        draw_sprite(r, s);
      }
    }
  }
}

} // namespace game2d