#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game2d {

void
generate_intersections(entt::registry& r,
                       const glm::ivec2& light_pos,
                       std::vector<std::tuple<float, float, float>>& intersections)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto camera_e = game2d::get_first<OrthographicCamera>(r);
  const auto& camera_t = r.get<TransformComponent>(camera_e);

  intersections.clear();

  const auto& solids_view = r.view<PhysicsSolidComponent, AABB>();

  // generate edges for all shapes
  int solids_count = 0;
  std::vector<std::array<glm::ivec2, 2>> edges;
  for (const auto& [e, solid, aabb] : solids_view.each()) {
    const auto& pos = aabb.center;
    const auto& size = aabb.size;
    if (size == glm::ivec2{ 0, 0 })
      continue;
    const float half_x = size.x / 2.0f;
    const float half_y = size.y / 2.0f;

    // Calculate edges for shape.
    const auto tl = glm::ivec2(pos.x - half_x, pos.y - half_y);
    const auto tr = glm::ivec2(pos.x + half_x, pos.y - half_y);
    const auto bl = glm::ivec2(pos.x - half_x, pos.y + half_y);
    const auto br = glm::ivec2(pos.x + half_x, pos.y + half_y);
    edges.push_back({ tl, bl }); // l
    edges.push_back({ tr, br }); // r
    edges.push_back({ tl, tr }); // u
    edges.push_back({ bl, br }); // d

    solids_count++;
  }
  if (solids_count == 0)
    return;

  const float epsilon = 0.0001f;
  const float radius = 150.0f;

  // add fake edges at the screen boundaries
  const glm::vec2 cp = { camera_t.position.x, camera_t.position.y };

  // note: the +5 is just so that it isnt "right" on the edge
  const float screen_w_half = (ri.viewport_size_render_at.x / 2.0f) + 5;
  const float screen_h_half = (ri.viewport_size_render_at.x / 2.0f) + 5;
  const glm::ivec2 tl = cp + glm::vec2(-screen_w_half, -screen_h_half);
  const glm::ivec2 tr = cp + glm::vec2(+screen_w_half, -screen_h_half);
  const glm::ivec2 bl = cp + glm::vec2(-screen_w_half, +screen_h_half);
  const glm::ivec2 br = cp + glm::vec2(+screen_w_half, +screen_h_half);
  edges.push_back({ tl, bl }); // l
  edges.push_back({ tr, br }); // r
  edges.push_back({ tl, tr }); // u
  edges.push_back({ bl, br }); // d

  for (const auto& e : edges) {
    for (int i = 0; i < 2; i++) {
      float rdx, rdy;
      rdx = (i == 0 ? e[0].x : e[1].x) - light_pos.x;
      rdy = (i == 0 ? e[0].y : e[1].y) - light_pos.y;

      const float base_ang = atan2f(rdy, rdx); // angle of ray vector
      float ang = 0.0f;
      for (int j = 0; j < 3; j++) {
        if (j == 0)
          ang = base_ang - epsilon;
        if (j == 1)
          ang = base_ang;
        if (j == 2)
          ang = base_ang + epsilon;

        // create ray along angle for required distance
        rdx = radius * cosf(ang);
        rdy = radius * sinf(ang);

        float min_t1 = INFINITY;
        float min_px = 0, min_py = 0, min_ang = 0;
        bool is_valid = false;

        // check for ray intersection with edges
        for (const auto& e2 : edges) {
          const float sdx = e2[1].x - e2[0].x;
          const float sdy = e2[1].y - e2[0].y;

          // make sure ray isn't colinear
          if (glm::abs(sdx - rdx) > 0.0f && glm::abs(sdy - rdy) > 0.0f) {
            // t2 is normalised distance from line segment start to line segment end of intersect point
            const float t2 = (rdx * (e2[0].y - light_pos.y) + (rdy * (light_pos.x - e2[0].x))) / (sdx * rdy - sdy * rdx);
            // t1 is normalised distance from source along ray to ray length of intersect point
            const float t1 = (e2[0].x + sdx * t2 - light_pos.x) / rdx;

            // If intersect point exists along ray, and along line
            // segment then intersect point is valid
            if (t1 > 0 && t2 >= 0 && t2 <= 1.0f) {
              // Check if this intersect point is closest to source. If
              // it is, then store this point and reject others
              if (t1 < min_t1) {
                min_t1 = t1;
                min_px = light_pos.x + rdx * t1;
                min_py = light_pos.y + rdy * t1;
                min_ang = atan2f(min_py - light_pos.y, min_px - light_pos.x);
                is_valid = true;
              }
            }
          }
        }
        if (is_valid)
          intersections.push_back({ min_ang, min_px, min_py });
      }
    }
  }

  // Sort the intersections by angle
  std::sort(intersections.begin(),
            intersections.end(),
            [&](const std::tuple<float, float, float>& t1, const std::tuple<float, float, float>& t2) {
              return std::get<0>(t1) < std::get<0>(t2);
            });

  // Remove duplicate (or similar) points from polygon
  const auto it = std::unique(intersections.begin(),
                              intersections.end(),
                              [&](const std::tuple<float, float, float>& t1, const std::tuple<float, float, float>& t2) {
                                return glm::abs(std::get<1>(t1) - std::get<1>(t2)) < 0.1f &&
                                       glm::abs(std::get<2>(t1) - std::get<2>(t2)) < 0.1f;
                              });

  intersections.resize(std::distance(intersections.begin(), it));
}

} // namespace game2d