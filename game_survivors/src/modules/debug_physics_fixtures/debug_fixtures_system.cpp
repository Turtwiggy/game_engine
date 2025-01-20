#include "debug_fixtures_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "modules/sprites/sprite_helpers.hpp"

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
        const float radius = circle->m_radius;
        const glm::vec2 pos = { body->GetPosition().x, body->GetPosition().y };

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
          s.col.a = 1.0f * 255;
          draw_sprite(r, s);

          prev_vert = cur_vert;
        }
      }

      else if (shape->GetType() == b2Shape::e_polygon) {
        continue; // disable polygon debugging

        const auto* polygon = static_cast<const b2PolygonShape*>(shape);

        const b2Transform transform = body->GetTransform();
        b2AABB aabb;
        polygon->ComputeAABB(&aabb, transform, 0);

        // Calculate size and position
        b2Vec2 size = aabb.upperBound - aabb.lowerBound;
        b2Vec2 center = 0.5f * (aabb.upperBound + aabb.lowerBound);

        // Create the sprite
        Sprite s;
        s.sprite = "EMPTY";
        s.pos = { center.x, center.y };
        s.size = { size.x, size.y };
        // s.col.a = 1.0f * 255;

        draw_sprite(r, s);
      }
    }
  }
}

} // namespace game2d