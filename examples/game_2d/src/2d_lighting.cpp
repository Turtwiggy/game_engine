// header
#include "2d_lighting.hpp"

// standard lib headers
#include <array>
#include <cmath>
#include <iostream>
#include <tuple>

// other project headers
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// engine project headers
#include "engine/maths_core.hpp"
#include "engine/opengl/util.hpp"

namespace game2d {

void
generate_intersections(GameObject2D& camera,
                       glm::ivec2& light_pos,
                       const std::vector<std::reference_wrapper<GameObject2D>>& ents,
                       const glm::ivec2& screen_wh,
                       std::vector<std::tuple<float, float, float>>& intersections)
{
  glm::vec2 light_world_pos = light_pos - camera.pos;
  intersections.clear();

  // generate edges for all shapes
  std::vector<std::array<glm::vec2, 2>> edges;

  for (auto& obj : ents) {
    const glm::vec2 pos = obj.get().pos;
    const glm::vec2 size = obj.get().render_size;
    // Calculate edges for shape
    // An edge consists of two points; the start and end
    glm::vec2 tl = pos;
    glm::vec2 tr = { pos.x + size.x, pos.y };
    glm::vec2 bl = { pos.x, pos.y + size.y };
    glm::vec2 br = { pos.x + size.x, pos.y + size.y };
    std::array<glm::vec2, 2> edge_l{ tl, bl };
    std::array<glm::vec2, 2> edge_r{ tr, br };
    std::array<glm::vec2, 2> edge_u{ tl, tr };
    std::array<glm::vec2, 2> edge_d{ bl, br };
    edges.push_back(edge_l);
    edges.push_back(edge_r);
    edges.push_back(edge_u);
    edges.push_back(edge_d);
  }

  const float epsilon = 0.0001f;
  const float radius = 150.0f;

  // add fake edges at the screen boundaries
  glm::vec2 tl = { 0.0f, 0.0f };
  glm::vec2 tr = { screen_wh.x, 0.0f };
  glm::vec2 bl = { 0.0f, screen_wh.y };
  glm::vec2 br = { screen_wh };
  std::array<glm::vec2, 2> screen_l{ tl, bl };
  std::array<glm::vec2, 2> screen_r{ tr, br };
  std::array<glm::vec2, 2> screen_u{ tl, tr };
  std::array<glm::vec2, 2> screen_d{ bl, br };
  edges.push_back(screen_l);
  edges.push_back(screen_r);
  edges.push_back(screen_u);
  edges.push_back(screen_d);

  // add fake edges around light to act as boundary cutoff
  // glm::vec2 tl = { light_pos.x - radius, light_pos.y - radius };
  // glm::vec2 tr = { light_pos.x + radius, light_pos.y - radius };
  // glm::vec2 bl = { light_pos.x - radius, light_pos.y + radius };
  // glm::vec2 br = { light_pos.x + radius, light_pos.y + radius };
  // std::array<glm::vec2, 2> screen_l{ tl, bl };
  // std::array<glm::vec2, 2> screen_r{ tr, br };
  // std::array<glm::vec2, 2> screen_u{ tl, tr };
  // std::array<glm::vec2, 2> screen_d{ bl, br };
  // edges.push_back(screen_l);
  // edges.push_back(screen_r);
  // edges.push_back(screen_u);
  // edges.push_back(screen_d);

  for (auto& e : edges) {
    for (int i = 0; i < 2; i++) {
      float rdx, rdy;
      rdx = (i == 0 ? e[0].x : e[1].x) - light_world_pos.x;
      rdy = (i == 0 ? e[0].y : e[1].y) - light_world_pos.y;

      float base_ang = atan2f(rdy, rdx); // angle of ray vector
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
        for (auto& e2 : edges) {
          float sdx = e2[1].x - e2[0].x;
          float sdy = e2[1].y - e2[0].y;

          // make sure ray isn't colinear
          if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f) {
            // t2 is normalised distance from line segment start to line segment end of intersect point
            float t2 =
              (rdx * (e2[0].y - light_world_pos.y) + (rdy * (light_world_pos.x - e2[0].x))) / (sdx * rdy - sdy * rdx);
            // t1 is normalised distance from source along ray to ray length of intersect point
            float t1 = (e2[0].x + sdx * t2 - light_world_pos.x) / rdx;

            // If intersect point exists along ray, and along line
            // segment then intersect point is valid
            if (t1 > 0 && t2 >= 0 && t2 <= 1.0f) {
              // Check if this intersect point is closest to source. If
              // it is, then store this point and reject others
              if (t1 < min_t1) {
                min_t1 = t1;
                min_px = light_world_pos.x + rdx * t1;
                min_py = light_world_pos.y + rdy * t1;
                min_ang = atan2f(min_py - light_world_pos.y, min_px - light_world_pos.x);
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

  // Remove duplicate (or simply similar) points from polygon
  auto it = std::unique(intersections.begin(),
                        intersections.end(),
                        [&](const std::tuple<float, float, float>& t1, const std::tuple<float, float, float>& t2) {
                          return std::fabs(std::get<1>(t1) - std::get<1>(t2)) < 0.1f &&
                                 std::fabs(std::get<2>(t1) - std::get<2>(t2)) < 0.1f;
                        });
  intersections.resize(std::distance(intersections.begin(), it));

  // bool show_debug_lines = true;
  // if (!show_debug_lines)
  //   return;

  // #ifdef WIN32
  // #ifdef _DEBUG

  //     // draw lines
  //     debug_line_shader.bind();
  //     debug_line_shader.set_vec4("colour", debug_line_shader_colour);

  //     glm::vec2 world_pos = gameobject_in_worldspace(camera, obj);
  //     glm::vec2 bl_pos = glm::vec2(world_pos.x, world_pos.y + obj.get().physics_size.y);
  //     glm::vec2 tr_pos = glm::vec2(world_pos.x + obj.get().physics_size.x, world_pos.y);
  //     bl_pos.x = fightingengine::scale(bl_pos.x, 0.0f, screen_wh.x, -1.0f, 1.0f);
  //     bl_pos.y = fightingengine::scale(bl_pos.y, 0.0f, screen_wh.y, 1.0f, -1.0f);
  //     tr_pos.x = fightingengine::scale(tr_pos.x, 0.0f, screen_wh.x, -1.0f, 1.0f);
  //     tr_pos.y = fightingengine::scale(tr_pos.y, 0.0f, screen_wh.y, 1.0f, -1.0f);

  //     glm::vec2 light_bl_pos = glm::vec2(light_world_pos.x, light_world_pos.y + light.physics_size.y);
  //     glm::vec2 light_tr_pos = glm::vec2(light_world_pos.x + light.physics_size.x, light_world_pos.y);
  //     light_bl_pos.x = fightingengine::scale(light_bl_pos.x, 0.0f, screen_wh.x, -1.0f, 1.0f);
  //     light_bl_pos.y = fightingengine::scale(light_bl_pos.y, 0.0f, screen_wh.y, 1.0f, -1.0f);
  //     light_tr_pos.x = fightingengine::scale(light_tr_pos.x, 0.0f, screen_wh.x, -1.0f, 1.0f);
  //     light_tr_pos.y = fightingengine::scale(light_tr_pos.y, 0.0f, screen_wh.y, 1.0f, -1.0f);

  //     glBegin(GL_LINE_LOOP);
  //     // line from light to tl
  //     glVertex2f(bl_pos.x, tr_pos.y);
  //     glVertex2f(light_bl_pos.x, light_tr_pos.y);
  //     // line from light to tr
  //     glVertex2f(tr_pos.x, tr_pos.y);
  //     glVertex2f(light_bl_pos.x, light_tr_pos.y);
  //     // line from light to bl
  //     glVertex2f(bl_pos.x, bl_pos.y);
  //     glVertex2f(light_bl_pos.x, light_tr_pos.y);
  //     // line from light to br
  //     glVertex2f(tr_pos.x, bl_pos.y);
  //     glVertex2f(light_bl_pos.x, light_tr_pos.y);
  //     glEnd();
  // #endif
  // #endif
}

} // namespace game2d