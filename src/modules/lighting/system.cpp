// // header
// #include "system.hpp"

// // my libs
// #include "modules/lighting/components.hpp"
// #include "modules/renderer/components.hpp"

// // std libs
// #include <array>
// #include <cmath>
// #include <tuple>

// // other libs
// #include <GL/glew.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>

// // engine libs
// #include "engine/maths/maths.hpp"
// #include "engine/opengl/util.hpp"

// void
// game2d::init_lighting_system(entt::registry& registry, engine::Application& app)
// {
//   auto lc = SINGLETON_LightingComponent();

//   // add a single light
//   lc.lights.push_back({ 100, 100 });

//   registry.ctx().emplace<SINGLETON_LightingComponent>(lc);
// }

// void
// game2d::update_lighting_system(entt::registry& registry, engine::Application& app, float dt)
// {
//   const auto& ri = registry.ctx().at<SINGLETON_RendererInfo>();
//   auto& lighting = registry.ctx().at<SINGLETON_LightingComponent>();
//   auto light_world_pos = lighting.lights[0]; // LIMITATION: one light
//   auto& intersections = lighting.intersections;
//   intersections.clear();

//   // generate edges for all shapes
//   std::vector<std::array<glm::ivec2, 2>> edges;

//   const auto& view = registry.view<PositionIntComponent, RenderSizeComponent>();
//   view.each([&edges](const auto& pos, const auto& size) {
//     // Calculate edges for shape
//     // An edge consists of two points; the start and end
//     glm::ivec2 tl = { pos.x, pos.y };
//     glm::ivec2 tr = { pos.x + size.w, pos.y };
//     glm::ivec2 bl = { pos.x, pos.y + size.h };
//     glm::ivec2 br = { pos.x + size.w, pos.y + size.h };
//     std::array<glm::ivec2, 2> edge_l{ tl, bl };
//     std::array<glm::ivec2, 2> edge_r{ tr, br };
//     std::array<glm::ivec2, 2> edge_u{ tl, tr };
//     std::array<glm::ivec2, 2> edge_d{ bl, br };
//     edges.push_back(edge_l);
//     edges.push_back(edge_r);
//     edges.push_back(edge_u);
//     edges.push_back(edge_d);
//   });

//   const float epsilon = 0.0001f;
//   const float radius = 150.0f;
//   const auto& screen_wh = ri.viewport_size_current;

//   // add fake edges around light to act as boundary cutoff
//   glm::ivec2 tl = { 0, 0 };
//   glm::ivec2 tr = { screen_wh.x, 0 };
//   glm::ivec2 bl = { 0, screen_wh.y };
//   glm::ivec2 br = { screen_wh };
//   std::array<glm::ivec2, 2> screen_l{ tl, bl };
//   std::array<glm::ivec2, 2> screen_r{ tr, br };
//   std::array<glm::ivec2, 2> screen_u{ tl, tr };
//   std::array<glm::ivec2, 2> screen_d{ bl, br };
//   edges.push_back(screen_l);
//   edges.push_back(screen_r);
//   edges.push_back(screen_u);
//   edges.push_back(screen_d);

//   for (auto& e : edges) {
//     for (int i = 0; i < 2; i++) {
//       float rdx = 0.0f;
//       float rdy = 0.0f;
//       rdx = (i == 0 ? e[0].x : e[1].x) - light_world_pos.x;
//       rdy = (i == 0 ? e[0].y : e[1].y) - light_world_pos.y;

//       float base_ang = atan2f(rdy, rdx); // angle of ray vector
//       float ang = 0.0f;

//       for (int j = 0; j < 3; j++) {
//         if (j == 0)
//           ang = base_ang - epsilon;
//         if (j == 1)
//           ang = base_ang;
//         if (j == 2)
//           ang = base_ang + epsilon;

//         // create ray along angle for required distance
//         rdx = radius * cosf(ang);
//         rdy = radius * sinf(ang);

//         float min_t1 = INFINITY;
//         float min_px = 0, min_py = 0, min_ang = 0;
//         bool is_valid = false;

//         // check for ray intersection with edges
//         for (auto& e2 : edges) {
//           float sdx = e2[1].x - e2[0].x;
//           float sdy = e2[1].y - e2[0].y;

//           // make sure ray isn't colinear
//           if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f) {
//             // t2 is normalised distance from line segment start to line segment end of intersect point
//             float t2 =
//               (rdx * (e2[0].y - light_world_pos.y) + (rdy * (light_world_pos.x - e2[0].x))) / (sdx * rdy - sdy *
//               rdx);
//             // t1 is normalised distance from source along ray to ray length of intersect point
//             float t1 = (e2[0].x + sdx * t2 - light_world_pos.x) / rdx;

//             // If intersect point exists along ray, and along line
//             // segment then intersect point is valid
//             if (t1 > 0 && t2 >= 0 && t2 <= 1.0f) {
//               // Check if this intersect point is closest to source. If
//               // it is, then store this point and reject others
//               if (t1 < min_t1) {
//                 min_t1 = t1;
//                 min_px = light_world_pos.x + rdx * t1;
//                 min_py = light_world_pos.y + rdy * t1;
//                 min_ang = atan2f(min_py - light_world_pos.y, min_px - light_world_pos.x);
//                 is_valid = true;
//               }
//             }
//           }
//         }
//         if (is_valid)
//           intersections.push_back({ min_ang, min_px, min_py });
//       }
//     }
//   }

//   // Sort the intersections by angle
//   std::sort(intersections.begin(),
//             intersections.end(),
//             [&](const std::tuple<float, float, float>& t1, const std::tuple<float, float, float>& t2) {
//               return std::get<0>(t1) < std::get<0>(t2);
//             });

//   // Remove duplicate (or simply similar) points from polygon
//   auto it = std::unique(intersections.begin(),
//                         intersections.end(),
//                         [&](const std::tuple<float, float, float>& t1, const std::tuple<float, float, float>& t2) {
//                           return std::fabs(std::get<1>(t1) - std::get<1>(t2)) < 0.1f &&
//                                  std::fabs(std::get<2>(t1) - std::get<2>(t2)) < 0.1f;
//                         });
//   intersections.resize(std::distance(intersections.begin(), it));
// };

// //
// // lighting fbo
// //

// //   Framebuffer::bind_fbo(resources.fbo_lighting);
// //   // Debugging; // Framebuffer::default_fbo(); glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
// //   RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
// //   RenderCommand::set_depth_testing(false);
// //   RenderCommand::clear();
// //   {
// //     // angle, x, y for intersections
// //     std::vector<std::tuple<float, float, float>> intersections;

// //     std::vector<std::reference_wrapper<GameObject2D>> lit_entities;
// //     lit_entities.insert(lit_entities.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
// //     lit_entities.insert(lit_entities.end(), gs.entities_player.begin(), gs.entities_player.end());
// //     lit_entities.insert(lit_entities.end(), gs.entities_trees.begin(), gs.entities_trees.end());

// //     std::vector<std::reference_wrapper<PointLight>> lights = {};
// //     for (auto& light : gs.point_lights) {
// //       if (light.cast_shadows) {
// //         lights.push_back(light);
// //       }
// //     }
// //     for (auto& l : lights) {
// //       GameObject2D& light = l.get().light_object;
// //       glm::ivec2 light_pos = light.pos - gs.camera.pos;

// //       // this generates collision from the light point to the entities
// //       generate_intersections(gs.camera, light_pos, lit_entities, screen_wh, intersections);

// //       // now create a triangle fan from the generated information
// //       triangle_fan_renderer::reset_quad_vert_count();
// //       triangle_fan_renderer::begin_batch();

// //       resources.fan_shader.bind();
// //       resources.fan_shader.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
// //       resources.fan_shader.set_mat4("projection", projection);
// //       { // render light
// //         triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, light_pos);
// //         for (int i = 0; i < intersections.size(); i++) {
// //           auto& intersection = intersections[i];
// //           glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
// //           triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, pos);
// //         }
// //         auto& intersection = intersections[0];
// //         glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
// //         triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, pos);
// //         triangle_fan_renderer::end_batch();
// //         triangle_fan_renderer::flush(resources.fan_shader);
// //       }
// //     }
// //   }

// //
// // in default_fbo
// //

// // r.instanced.set_float("time", app.seconds_since_launch);
// // r.instanced.set_int("screen_w", screen_wh.x);
// // r.instanced.set_int("screen_h", screen_wh.y);
// // r.instanced.set_bool("do_spritesheet", false);

// // send light positions to shader
// // {
// //   const int max_lights = 32;
// //   for (int i = 0; i < gs.point_lights.size(); i++) {
// //     if (i >= max_lights) {
// //       std::cout << "warning: too many lights" << "\n";
// //       continue;
// //     }
// //     auto& light = gs.point_lights[i];
// //     glm::vec2 light_pos = light.light_object.pos;
// //     light_pos = glm::vec3(light_pos.x, glm::abs(light_pos.y - screen_wh.y), 0.0f); // flip
// //     resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(i) + "]", true);
// //     resources.instanced_quad_shader.set_vec3("light_pos[" + std::to_string(i) + "]",
// //                                               glm::vec3(light_pos, 0.0f));
// //     resources.instanced_quad_shader.set_float("light_linear[" + std::to_string(i) + "]", light.linear);
// //     resources.instanced_quad_shader.set_float("light_quadratic[" + std::to_string(i) + "]", light.quadratic);
// //   }
// //   for (int j = max_lights - 1; j >= i; j--) {
// //     resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(j) + "]", false);
// //   }
// // }
