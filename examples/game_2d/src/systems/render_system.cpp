// your header
#include "systems/render_system.hpp"

// game headers
#include "constants.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hoverable.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"

// helpers
#include "helpers/renderers/batch_sprite.hpp"
#include "helpers/renderers/batch_triangle_fan.hpp"
#include "helpers/spritemap.hpp"

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
using namespace engine;

// void
// toggle_fullscreen(Application& app,
//                   glm::ivec2& screen_wh,
//                   glm::mat4& projection,
//                   unsigned int tex_id_lighting,
//                   unsigned int tex_id_main_scene)
// {
//   app.get_window().toggle_fullscreen(); // SDL2 window toggle
//   screen_wh = app.get_window().get_size();
//   RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
//   projection = glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

//   update_texture_size(screen_wh, tex_id_lighting);
//   update_texture_size(screen_wh, tex_id_main_scene);
// }

void
game2d::init_render_system(entt::registry& registry, const glm::ivec2& screen_wh)
{
  Resources r;
  r.fbo_main_scene = Framebuffer::create_fbo();
  r.fbo_lighting = Framebuffer::create_fbo();
  r.tex_id_main_scene = create_texture(screen_wh, tex_unit_main_scene, r.fbo_main_scene);
  r.tex_id_lighting = create_texture(screen_wh, tex_unit_lighting, r.fbo_lighting);
  r.instanced = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
  r.fan = Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");

  // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#context-variables
  registry.set<Resources>(r);

  // initialize renderers
  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
  print_gpu_info();
  sprite_renderer::SpriteBatchRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  static glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);
  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  r.instanced.bind();
  r.instanced.set_mat4("projection", projection);
  r.instanced.set_int_array("textures", textures, 3);
  r.instanced.set_int("screen_w", screen_wh.x);
  r.instanced.set_int("screen_h", screen_wh.y);
};

void
game2d::update_render_system(entt::registry& registry)
{
  Resources& r = registry.ctx<Resources>();

  Framebuffer::default_fbo();
  glm::vec4 background_colour = glm::vec4(1.0f);
  RenderCommand::set_clear_colour(background_colour);
  RenderCommand::clear();

  sprite_renderer::SpriteBatchRenderer::reset_quad_vert_count();
  sprite_renderer::SpriteBatchRenderer::begin_batch();

  sprite_renderer::RenderDescriptor desc;

  auto view = registry.view<const Position, const Size, const Colour, const Sprite>();

  view.each([&registry, &r, &desc](const auto entity, const auto& p, const auto& s, const auto& c, const auto& spr) {
    desc.pos_tl = { p.x - int(s.w / 2.0f), p.y - int(s.h / 2.0f) };
    desc.colour = c.colour;
    desc.size = { s.w, s.h };
    desc.tex_slot = tex_unit_kenny_nl;
    desc.sprite_offset = sprite::spritemap::get_sprite_offset(spr.sprite);
    desc.angle_radians = 0.0f;

    // (optional) hoverable component
    bool has_hoverable = registry.all_of<Hoverable>(entity);
    if (has_hoverable) {
      auto& hoverable = registry.get<Hoverable>(entity);
      if (hoverable.mouse_is_hovering) {
        // int y_offset = -3;
        int y_offset = 0;
        desc.pos_tl.y += y_offset;
      }
    }

    sprite_renderer::SpriteBatchRenderer::draw_sprite(desc, r.instanced);
  });

  sprite_renderer::SpriteBatchRenderer::end_batch();
  sprite_renderer::SpriteBatchRenderer::flush(r.instanced);
};

//
// lighting fbo
//

//   Framebuffer::bind_fbo(resources.fbo_lighting);
//   // Debugging; // Framebuffer::default_fbo(); glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//   RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
//   RenderCommand::set_depth_testing(false);
//   RenderCommand::clear();
//   {
//     // angle, x, y for intersections
//     std::vector<std::tuple<float, float, float>> intersections;

//     std::vector<std::reference_wrapper<GameObject2D>> lit_entities;
//     lit_entities.insert(lit_entities.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
//     lit_entities.insert(lit_entities.end(), gs.entities_player.begin(), gs.entities_player.end());
//     lit_entities.insert(lit_entities.end(), gs.entities_trees.begin(), gs.entities_trees.end());

//     std::vector<std::reference_wrapper<PointLight>> lights = {};
//     for (auto& light : gs.point_lights) {
//       if (light.cast_shadows) {
//         lights.push_back(light);
//       }
//     }
//     for (auto& l : lights) {
//       GameObject2D& light = l.get().light_object;
//       glm::ivec2 light_pos = light.pos - gs.camera.pos;

//       // this generates collision from the light point to the entities
//       generate_intersections(gs.camera, light_pos, lit_entities, screen_wh, intersections);

//       // now create a triangle fan from the generated information
//       triangle_fan_renderer::reset_quad_vert_count();
//       triangle_fan_renderer::begin_batch();

//       resources.fan_shader.bind();
//       resources.fan_shader.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
//       resources.fan_shader.set_mat4("projection", projection);
//       { // render light
//         triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, light_pos);
//         for (int i = 0; i < intersections.size(); i++) {
//           auto& intersection = intersections[i];
//           glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
//           triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, pos);
//         }
//         auto& intersection = intersections[0];
//         glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
//         triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, pos);
//         triangle_fan_renderer::end_batch();
//         triangle_fan_renderer::flush(resources.fan_shader);
//       }
//     }
//   }
//   //
//   // main scene fbo
//   //
//   Framebuffer::bind_fbo(resources.fbo_main_scene);
//   RenderCommand::set_clear_colour(background_colour);
//   RenderCommand::set_depth_testing(false);
//   RenderCommand::clear();
//   {
//     sprite_renderer::reset_quad_vert_count();
//     sprite_renderer::begin_batch();
//     resources.instanced_quad_shader.bind();
//     resources.instanced_quad_shader.set_float("time", app.seconds_since_launch);
//     resources.instanced_quad_shader.set_mat4("projection", projection);
//     resources.instanced_quad_shader.set_bool("do_lighting", false);
//     resources.instanced_quad_shader.set_bool("do_spritesheet", true);

//     if (gs.game_running == GameRunning::ACTIVE || gs.game_running == GameRunning::PAUSED ||
//         gs.game_running == GameRunning::GAME_OVER) {

//       // This list defines the render order
//       std::vector<std::reference_wrapper<GameObject2D>> renderables;
//       renderables.insert(renderables.end(), gs.entities_vfx.begin(), gs.entities_vfx.end());
//       renderables.insert(renderables.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
//       renderables.insert(renderables.end(), gs.entities_bullets.begin(), gs.entities_bullets.end());
//       renderables.insert(renderables.end(), gs.entities_player.begin(), gs.entities_player.end());
//       renderables.push_back(gs.weapon_shovel);
//       renderables.push_back(gs.weapon_pistol);
//       renderables.push_back(gs.weapon_shotgun);
//       renderables.push_back(gs.weapon_machinegun);
//       renderables.insert(renderables.end(), gs.entities_trees.begin(), gs.entities_trees.end());
//       renderables.push_back(pushable);

//       for (auto& obj : renderables) {
//         if (!obj.get().do_render)
//           continue;
//         sprite_renderer::draw_instanced_sprite(gs.camera, screen_wh, resources.instanced_quad_shader, obj.get());
//       }

//       sprite_renderer::draw_sprites_debug(
//         gs.camera, screen_wh, renderables, resources.colour_shader, debug_line_colour);

//     } // <!-- end GameRunning::Active -->
//     sprite_renderer::end_batch();
//     sprite_renderer::flush(resources.instanced_quad_shader);
//   } // <!-- end main scene fbo -->

//   //
//   // default fbo, draw a texture to a quad
//   //
//   Framebuffer::default_fbo();
//   RenderCommand::set_clear_colour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // hit damage colour
//   RenderCommand::set_depth_testing(false);
//   RenderCommand::clear();
//   {
//     sprite_renderer::reset_quad_vert_count();
//     sprite_renderer::begin_batch();
//     resources.instanced_quad_shader.bind();
//     resources.instanced_quad_shader.set_float("time", app.seconds_since_launch);
//     glm::mat4 flip = glm::mat4(1.0f); // flip because opengl textures have different axis
//     flip = glm::rotate(flip, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//     resources.instanced_quad_shader.set_mat4("projection", flip * projection);
//     resources.instanced_quad_shader.set_bool("do_lighting", true);
//     resources.instanced_quad_shader.set_int("screen_w", screen_wh.x);
//     resources.instanced_quad_shader.set_int("screen_h", screen_wh.y);
//     resources.instanced_quad_shader.set_bool("do_spritesheet", false);
//     { // send light positions to shader
//       const int max_lights = 32;
//       int i = 0;
//       for (i = 0; i < gs.point_lights.size(); i++) {
//         if (i >= max_lights) {
//           std::cout << "warning: too many lights" << std::endl;
//           continue;
//         }
//         auto& light = gs.point_lights[i];
//         glm::vec2 light_pos = light.light_object.pos;
//         light_pos = glm::vec3(light_pos.x, glm::abs(light_pos.y - screen_wh.y), 0.0f); // flip

//         resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(i) + "]", true);
//         resources.instanced_quad_shader.set_vec3("light_pos[" + std::to_string(i) + "]",
//                                                  glm::vec3(light_pos, 0.0f));
//         resources.instanced_quad_shader.set_float("light_linear[" + std::to_string(i) + "]", light.linear);
//         resources.instanced_quad_shader.set_float("light_quadratic[" + std::to_string(i) + "]", light.quadratic);
//       }
//       for (int j = max_lights - 1; j >= i; j--) {
//         resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(j) + "]", false);
//       }
//     }

//     { // draw single quad as entire screen
//       GameObject2D screen_object = gameobject::create_generic();
//       screen_object.sprite = sprite::type::SQUARE;
//       screen_object.render_size = glm::vec2(screen_wh.x, screen_wh.y);
//       screen_object.tex_slot = tex_unit_main_scene;
//       sprite_renderer::draw_instanced_sprite(gs.camera, screen_wh, resources.instanced_quad_shader, screen_object);
//     }
//     sprite_renderer::end_batch();
//     sprite_renderer::flush(resources.instanced_quad_shader);
//     // CHECK_OPENGL_ERROR(0);
//   }
// }