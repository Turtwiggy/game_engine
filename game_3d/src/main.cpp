#define _CRT_SECURE_NO_WARNINGS

// c++ standard library headers
#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>
#include <thread>
#include <vector>

// other library headers
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// your project headers
#include "engine/application.hpp"
#include "engine/camera.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/model.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/renderer.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/triangle.hpp"
#include "engine/opengl/util.hpp"
#include "engine/tools/profiler.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

//
// game state
//
const int STARTING_CUBES = 35;
glm::vec3 player_pos(0.0f, 0.0f, 0.0f);

// Util function to log time since start of the program
void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::cout << label << y << "ms" << std::endl;
}

int
main(int argc, char** argv)
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  uint32_t width = 1280;
  uint32_t height = 720;
  Application app("Fighting Game!", width, height);
  // app.set_fps_limit(120.0f);
  // app.get_window().set_fullscreen(true);
  // app.remove_fps_limit();
  log_time_since("starting app... ", app_start);

  Camera camera;
  camera.set_perspective(glm::radians(65.0f), (float)width / (float)height, 0.1f, 100.0f);
  camera.target_yaw = -116.0f;
  camera.target_pitch = -7.8f;
  camera.target_position = glm::vec3(2.1f, 1.3f, 2.6f);

  RandomState rnd;
  Profiler profiler;

  // Renderer renderer(rnd);
  RenderCommand::init();
  RenderCommand::set_clear_colour({ 0.9f, 0.9f, 0.9f, 1.0f });
  RenderCommand::set_depth_testing(true);

  //
  // TODO sound
  //

  //
  // load shaders
  //

  Shader texture_shader = Shader("lit.vert", "basic_shader.frag");

  log_time_since("shaders loaded ", app_start);

  //
  // load models (note, pretty slow at the moment. could thread)
  //

  // Model model_1("assets/models/cyborg/cyborg.obj");
  Model model_2("assets/models/rpg_characters_nov_2020/OBJ/Monk.obj");

  log_time_since("models loaded ", app_start);

  // Shader basicTextureShader = Shader::create("angrygl/basic_texture_shader.vert", "angrygl/floor_shader.frag");
  // basicTextureShader.use();
  // basicTextureShader.setVec3("directionLight.dir", lightDir);
  // basicTextureShader.setVec3("directionLight.color", floorLightColor);
  // basicTextureShader.setVec3("ambient", floorAmbientColor);

  // Shader instancedTextureShader =
  // Shader::create("angrygl/instanced_texture_shader.vert", "angrygl/basic_texture_shader.frag");
  // Shader nodeShader = Shader::create("angrygl/redshader.vert", "angrygl/redshader.frag");
  // nodeShader.use();

  // wigglyShader.use();
  // wigglyShader.use();
  // wigglyShader.setInt("texture_diffuse", texUnit_wigglyBoi);
  // wigglyShader.setVec3("directionLight.dir", playerLightDir);
  // wigglyShader.setVec3("directionLight.color", lightColor);
  // wigglyShader.setVec3("ambient", ambientColor);

  //
  // enemies
  //

  // std::vector<Enemy> enemies;
  // EnemySpawner enemySpawner(monsterY, &enemies);

  //
  // random cubes
  //

  std::vector<glm::vec3> cube_pos;
  glm::vec3 rand_pos{ 0.0f, 0.0f, 0.0f };
  for (int i = 0; i < STARTING_CUBES; i++) {
    rand_pos.x = rand_det_s(rnd.rng, -10.0f, 10.0f);
    rand_pos.y = rand_det_s(rnd.rng, -10.0f, 10.0f);
    rand_pos.z = rand_det_s(rnd.rng, -10.0f, 10.0f);
    cube_pos.push_back(rand_pos);
  }

  // App

  while (app.is_running()) {
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin();

    float delta_time_s = app.get_delta_time();

    profiler.begin(Profiler::Stage::SdlInput);

    app.poll(); // input events

    //
    // Settings: Shutdown app
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
      app.shutdown();

    //
    // Settings: Toggle mouse capture
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
      app.get_window().toggle_mouse_capture();

    //
    // Settings: Fullscreen
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f)) {
      app.get_window().toggle_fullscreen(); // SDL2 window toggle
      glm::ivec2 screen_size = app.get_window().get_size();
      RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
    }

    //
    // Process Input: Keyboard
    //
    camera_fly_around(camera, delta_time_s, app.get_input().get_keyboard_state());
    // camera_follow_position(camera, player_pos, delta_time_s);

    //
    // Process Input: Mouse
    //
    if (app.get_window().get_mouse_captured()) {
      glm::ivec2 rel_mouse = app.get_window().get_relative_mouse_position();
      camera.process_mouse_input(static_cast<float>(rel_mouse.x), static_cast<float>(rel_mouse.y));
    }

    //
    // Shader hot reloading
    //
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_r)) {
      reload_shader_program(&texture_shader.ID, "lit.vert", "basic_shader.frag");
    }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    //
    // Game State Tick
    //

    camera.update(delta_time_s);

    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    //
    // Rendering
    //

    // renderer.update(delta_time_s, camera, rnd, cube_pos, app.get_window().get_size());

    // RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
    RenderCommand::clear();
    glm::mat4 view_projection = camera.get_view_projection_matrix();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));

    texture_shader.bind();
    texture_shader.set_mat4("view_projection", view_projection);
    texture_shader.set_mat4("model", model);
    model_2.draw(texture_shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    texture_shader.set_mat4("model", model);
    model_2.draw(texture_shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));
    texture_shader.set_mat4("model", model);
    model_2.draw(texture_shader);

    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    //
    // GUI
    //

    app.gui_begin();

    // ImGui::Begin("Depth Texture");
    // Using a Child allow to fill all the space of the window.
    // ImGui::BeginChild("Depth Texture");
    // ImVec2 wsize = ImGui::GetWindowSize();
    // ImGui::Image(
    //   (ImTextureID)renderer.shadowmapping_pass.depthmap_tex, ImVec2(wsize.x, wsize.y), ImVec2(0, 1), ImVec2(1, 0));
    // // ImGui::Text("Depth Texture being rendererd");
    // ImGui::EndChild();
    // ImGui::End();

    ImGui::Begin("Camera");
    {
      ImGui::Text("Camera Pos: %f %f %f", camera.get_position().x, camera.get_position().y, camera.get_position().z);
      ImGui::Text("Camera Pitch: %f", camera.get_pitch());
      ImGui::Text("Camera Yaw: %f", camera.get_yaw());
    }
    ImGui::End();

    // ImGui::Begin("Renderer", (bool*)1);
    // ImGui::Text("Draw calls: %i", renderer.draw_calls);
    // if (ImGui::CollapsingHeader("Post-processing"))
    // {
    //     ImGui::Checkbox("Vignette", &renderer->GetPostProcessor()->Vignette);
    //     ImGui::Checkbox("SSAO", &renderer->GetPostProcessor()->SSAO);
    //     ImGui::Checkbox("Bloom", &renderer->GetPostProcessor()->Bloom);
    //     //ImGui::Checkbox("SSR", &renderer->GetPostProcessor()->SSR);
    //     //ImGui::Checkbox("TXAA", &renderer->GetPostProcessor()->TXAA);
    //     ImGui::Checkbox("Sepia", &renderer->GetPostProcessor()->Sepia);
    //     ImGui::Checkbox("Motion Blur",
    //     &renderer->GetPostProcessor()->MotionBlur);
    // }
    // ImGui::End();

    if (ImGui::BeginMainMenuBar()) {

      if (ImGui::MenuItem("Quit", "Esc")) {
        app.shutdown();
      }

      ImGui::SameLine(ImGui::GetWindowWidth() - 154.0f);
      ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

      ImGui::EndMainMenuBar();
    }

    profiler_panel::draw(profiler, delta_time_s);

    app.gui_end();

    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);

    app.frame_end(delta_time_s);

    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }

  return 0;
}

// Below code is converting mouse position in to ray
// Note: Could be faster to do a simple line
// intersection from camera pos
// to plane along the direction vector.
// Probably faster too, as there's no matrix inverse
// -------------------------------------------------

// {
//     //if mouse is clicked
//     if( app.get_input().get_mouse_lmb_held() )
//     {
//         //This is the lazy implementation of object picking!
//         //our ray origin is the camera position
//         glm::vec3 ray_origin = camera.Position;

//         //step 1: get mouse pos
//         //range [0:width, height:0]
//         glm::ivec2 abs_mouse = app.get_window().get_mouse_position();

//         //printf("mouse x: %i y: %i \n", abs_mouse.x, abs_mouse.y);

//         //step 2: convert mouse position in to 3d normalized device
//         coordinates
//         //range [-1:1, -1:1, -1:1]
//         float x = (2.0f * abs_mouse.x) / width - 1.0f;
//         float y = 1.0f - (2.0f * abs_mouse.y) /  height;
//         glm::vec2 normalized_device_coords {x, y};

//         //step 3: convert normalized device coords to homogenous clip coords
//         //range [-1:1, -1:1, -1:1, -1:1]
//         //note: ray points in the -z direction
//         glm::vec4 ray_clip {normalized_device_coords.x, normalized_device_coords.y, -1.0, 1.0};

//         //Convert homogenous clip coords to  eye camera coordinates
//         //range [-x:x, -y:y, -z:z, -w:w]
//         glm::vec4 eye_ray = glm::inverse(camera.Projection) * ray_clip;
//         eye_ray.z = -1.0f;
//         eye_ray.w = 0.0f;

//         //step 5: convert eye camera coordinates to world coordinates
//         //range [-x:x, -y:y, -z:z, -w:w]
//         glm::vec3 ray_world = glm::vec3(glm::inverse(camera.View) * eye_ray);
//         ray_world = glm::normalize(ray_world);

//         printf("ray x: %f y: %f z: %f \n", ray_world.x, ray_world.y,
//         ray_world.z);
//     }
// }

// ai for chasing player
// ---------------------

// void
// chase_player(const float delta_time, std::vector<Enemy>* enemies)
// {
//   const glm::vec3 playerCollisionPosition(playerPosition.x, monsterY, playerPosition.z);
//   for (int i = 0; i < enemies->size(); ++i) {
//     auto& e = (*enemies)[i];
//     glm::vec3 dir = playerPosition - e.position;
//     dir.y = 0.0f;
//     e.dir = glm::normalize(dir);
//     e.position += e.dir * delta_time * monster_speed;
//     if (isAlive) {
//       const glm::vec3 p1 = e.position - e.dir * (ENEMY_COLLIDER.height / 2);
//       const glm::vec3 p2 = e.position + e.dir * (ENEMY_COLLIDER.height / 2);
//       const float dist = distanceBetweenPointAndLineSegment(playerCollisionPosition, p1, p2);
//       if (dist <= (playerCollisionRadius + ENEMY_COLLIDER.radius)) {
//         std::cout << "GOTTEM!" << std::endl;
//         isAlive = false;
//         playerMovementDir = glm::vec2(0.0f, 0.0f);
//       }
//     }
//   }
// }

// THIS IS FOR A FIXED GAME TICK
// -----------------------------
// seconds_since_last_game_tick += delta_time_in_seconds;
// while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
//{
//    //Fixed update
//    fixed_tick(SECONDS_PER_FIXED_TICK);
//    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
//}

// load textures
// -------------

// const int tex_unit_octopus_diffuse = 0;
// const int tex_unit_container_diffuse = 1;

// std::cout << "loading textures... " << std::endl;
// {
//   std::vector<std::pair<int, std::string>> textures_to_load;
//   textures_to_load.emplace_back(tex_unit_octopus_diffuse, "assets/textures/octopus.png");
//   textures_to_load.emplace_back(tex_unit_container_diffuse, "assets/textures/container.jpg");

//   std::vector<std::thread> threads;
//   std::vector<StbLoadedTexture> loaded_textures(textures_to_load.size());

//   for (int i = 0; i < textures_to_load.size(); ++i) {
//     const std::pair<int, std::string>& tex_to_load = textures_to_load[i];
//     threads.emplace_back([&tex_to_load, i, &loaded_textures]() {
//       loaded_textures[i] = load_texture(tex_to_load.second, tex_to_load.first);
//     });
//   }
//   for (auto& thread : threads) {
//     thread.join();
//   }
//   for (StbLoadedTexture& l : loaded_textures) {
//     Texture2D tex;
//     tex.generate(l);
//     tex.bind(l.texture_unit);
//  stbi_image_free(stb_tex.data);

//   }
// }
// log_time_since("textures loaded ", app_start);
