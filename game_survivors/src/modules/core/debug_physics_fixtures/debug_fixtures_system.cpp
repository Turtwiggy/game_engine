#include "pch.hpp"

#include "debug_fixtures_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"

namespace game2d {

void
update_debug_fixtures_system(entt::registry& r)
{
  const auto& physics_c = get_first_component<SINGLE_Physics>(r);

  const auto view = r.view<const TransformComponent, const PhysicsBodyComponent, const TagComponent>();
  for (const auto& [e, t_c, body_c, tag_c] : view.each()) {
    const auto body_id = body_c.bodyId;

    int count = b2Body_GetShapeCount(body_id);
    std::vector<b2ShapeId> array;
    array.resize(count);
    b2Body_GetShapes(body_id, array.data(), count);

    for (int i = 0; const auto shape_id : array) {
      const auto type = b2Shape_GetType(shape_id);
      if (type == b2_circleShape) {

        const auto circle = b2Shape_GetCircle(shape_id);
        const auto pos_m = circle.center;
        const auto rad_m = circle.radius;
        const auto pos = meters_to_pixels(pos_m) + glm::vec2{ t_c.position.x, t_c.position.y };
        const auto rad = meters_to_pixels(rad_m);

        constexpr int segments = 32;
        constexpr float angle_step = engine::TWO_PI / segments;
        glm::vec2 prev_vert = pos + glm::vec2(rad, 0);
        for (int i = 1; i <= segments; i++) {
          const auto angle = i * angle_step;
          const auto cur_vert = pos + rad * glm::vec2{ cos(angle), sin(angle) };
          const auto line = generate_line(cur_vert, prev_vert, 2.0f);
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

      // assume box
      else if (type == b2_polygonShape) {

        const auto polygon = b2Shape_GetPolygon(shape_id);
        const auto center_m = polygon.centroid;
        const auto aabb_m = b2Shape_GetAABB(shape_id);
        const auto size_m = aabb_m.upperBound - aabb_m.lowerBound;
        const auto pos_m = b2Body_GetPosition(body_id);
        const auto rotation = b2Rot_GetAngle(b2Body_GetRotation(body_c.bodyId));

        const auto verts = polygon.vertices;
        for (int i = 0; i < polygon.count; i++) {
          const int32 cur_idx = i;
          const int32 nxt_idx = (i + 1) % polygon.count; // Wrap around to first vertex

          // Rotate vertices around the body's position
          b2Vec2 rel_v0 = verts[cur_idx] - polygon.centroid;
          b2Vec2 rel_v1 = verts[nxt_idx] - polygon.centroid;
          float cos_r = cos(rotation);
          float sin_r = sin(rotation);
          b2Vec2 rot_v0 = { rel_v0.x * cos_r - rel_v0.y * sin_r, rel_v0.x * sin_r + rel_v0.y * cos_r };
          b2Vec2 rot_v1 = { rel_v1.x * cos_r - rel_v1.y * sin_r, rel_v1.x * sin_r + rel_v1.y * cos_r };
          b2Vec2 v0 = pos_m + polygon.centroid + rot_v0;
          b2Vec2 v1 = pos_m + polygon.centroid + rot_v1;
          const auto line = generate_line(meters_to_pixels(v0), meters_to_pixels(v1), 4.0f);

          Sprite s;
          s.sprite = "EMPTY";
          s.pos = line.position;
          s.size = line.scale;
          s.z_rotation = line.rotation;
          s.col.a = static_cast<int>(0.5f * 255);
          s.z_idx = ZLayer::BACKGROUND;
          draw_sprite(r, s);
        }

        // Create the sprite
        // const auto col = (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f);
        // Sprite s;
        // s.sprite = "EMPTY";
        // s.pos = meters_to_pixels(center_m) + meters_to_pixels(pos_m);
        // s.size = meters_to_pixels(size_m);
        // s.col = engine::SRGBColour{ col.x, col.y, col.z, col.w };
        // s.z_rotation = rotation;
        // s.z_idx = ZLayer::BACKGROUND;
        // draw_sprite(r, s);

      } else if (type == b2_chainSegmentShape) {

        const auto chain = b2Shape_GetChainSegment(shape_id);
        const auto segment = chain.segment;
        const auto line = generate_line(meters_to_pixels(segment.point1), meters_to_pixels(segment.point2), 10.0f);

        Sprite s;
        s.sprite = "EMPTY";
        s.pos = line.position;
        s.size = line.scale;
        s.z_rotation = line.rotation;
        s.col.a = 0.5f * 255;
        s.z_idx = ZLayer::BACKGROUND;
        draw_sprite(r, s);

      } else {
        // what shape?
        int k = 1;
      }

      i++;
    }
  }
}

} // namespace game2d