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

//
// in default_fbo
//

// r.instanced.set_float("time", app.seconds_since_launch);
// r.instanced.set_int("screen_w", screen_wh.x);
// r.instanced.set_int("screen_h", screen_wh.y);
// r.instanced.set_bool("do_spritesheet", false);

// send light positions to shader
// {
//   const int max_lights = 32;
//   for (int i = 0; i < gs.point_lights.size(); i++) {
//     if (i >= max_lights) {
//       std::cout << "warning: too many lights" << std::endl;
//       continue;
//     }
//     auto& light = gs.point_lights[i];
//     glm::vec2 light_pos = light.light_object.pos;
//     light_pos = glm::vec3(light_pos.x, glm::abs(light_pos.y - screen_wh.y), 0.0f); // flip
//     resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(i) + "]", true);
//     resources.instanced_quad_shader.set_vec3("light_pos[" + std::to_string(i) + "]",
//                                               glm::vec3(light_pos, 0.0f));
//     resources.instanced_quad_shader.set_float("light_linear[" + std::to_string(i) + "]", light.linear);
//     resources.instanced_quad_shader.set_float("light_quadratic[" + std::to_string(i) + "]", light.quadratic);
//   }
//   for (int j = max_lights - 1; j >= i; j--) {
//     resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(j) + "]", false);
//   }
// }
