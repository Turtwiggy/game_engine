#pragma once

// Shader hot reloading
// if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
//   reload_shader_program(&fun_shader.ID, "2d_texture.vert", "effects/posterized_water.frag");
//   fun_shader.bind();
//   fun_shader.set_mat4("projection", projection);
//   fun_shader.set_int("tex", tex_unit_kenny_nl);
// }

// float mousewheel = app.get_input().get_mousewheel_y();
// float epsilon = 0.0001f;
// if (mousewheel > epsilon || mousewheel < -epsilon) {
//   // PHYSICS_GRID_SIZE += static_cast<int>(mousewheel);
//   // PHYSICS_GRID_SIZE = glm::max(PHYSICS_GRID_SIZE, 0);
//   float val = (mousewheel * 10.0f);
//   for (auto& p : entities_player) {
//     p.size = p.size + val;
//   }
// }