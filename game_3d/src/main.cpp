#define _CRT_SECURE_NO_WARNINGS

// c++ standard library headers
#include <chrono>
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
#include "engine/camera/fly_camera.hpp"
#include "engine/core/application.hpp"
#include "engine/maths/random.hpp"
#include "engine/opengl/model.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/renderer.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/triangle.hpp"
#include "engine/opengl/util.hpp"
#include "engine/tools/profiler.hpp"
#include "engine/ui/profiler_panel.hpp"
using namespace fightingengine;

// texture units
// const int texUnit_playerDiffuse = 0;
// const int texUnit_gunDiffuse = 1;
// const int texUnit_floorDiffuse = 2;
// const int texUnit_wigglyBoi = 3;
// const int texUnit_bullet = 4;
// const int texUnit_floorNormal = 5;
// const int texUnit_playerNormal = 6;
// const int texUnit_gunNormal = 7;
// const int texUnit_shadowMap = 8;
// const int texUnit_emissionFBO = 9;
// const int texUnit_playerEmission = 10;
// const int texUnit_gunEmission = 11;
// const int texUnit_scene = 12;
// const int texUnit_horzBlur = 13;
// const int texUnit_vertBlur = 14;
// const int texUnit_impactSpriteSheet = 15;
// const int texUnit_muzzleFlashSpriteSheet = 16;
// const int texUnit_floorSpec = 18;
// const int texUnit_playerSpec = 19;
// const int texUnit_gunSpec = 20;

// player
// glm::vec3 player_pos = glm::vec3(0.0f, 0.0f, 0.0f);
// glm::vec2 player_mov_dir = glm::vec2(0.0f, 0.0f);
// float lastFireTime = 0.0f;
// bool isTryingToFire = false;
// const float fireInterval = 0.1f; // seconds
// const int spreadAmount = 20;
// const float playerSpeed = 1.5f;
// const float playerCollisionRadius = 0.35f;
// bool isAlive = true;
// float aimTheta = 0.0f;

// models
// const float playerModelScale = 0.0044f;
// const float playerModelGunHeight = 120.0f;       // un-scaled
// const float playerModelGunMuzzleOffset = 100.0f; // un-scaled
// const float monsterY = playerModelScale * playerModelGunHeight;

// const glm::vec3 light_dir = glm::normalize(glm::vec3(-0.8f, 0.0f, -1.0f));
// const glm::vec3 player_light_dir = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
// const float light_factor = 0.8;
// const float non_blue = 0.9f;
// const glm::vec3 light_colour =
//   lightFactor * 1.0f * glm::vec3(non_blue * 0.406f, non_blue * 0.723f, 1.0f);
// const glm::vec3 ambient_colour =
//   lightFactor * 0.10f * glm::vec3(non_blue * 0.7f, non_blue * 0.7f, 0.7f);

// const float florrLightFactor = 0.35;
// const float floorNonBlue = 0.7f;
// const glm::vec3 floorLightColor =
//   florrLightFactor * 1.0f * glm::vec3(floorNonBlue * 0.406f, floorNonBlue * 0.723f, 1.0f);
// const glm::vec3 floorAmbientColor =
//   florrLightFactor * 0.50f * glm::vec3(floorNonBlue * 0.7f, floorNonBlue * 0.7f, 0.7f);

// enemies
const float monster_speed = 0.6f;

// game state
int desired_cubes = 35;

// ai

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

  glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
  FlyCamera camera{ camera_pos };
  camera.SetPerspective(glm::radians(65.0f), (float)width / (float)height, 0.1f, 100.0f);

  RandomState rnd;
  Profiler profiler;
  ProfilerPanel profiler_panel;

  // Renderer renderer(rnd);
  RenderCommand::init();
  RenderCommand::set_clear_colour({ 0.1f, 0.1f, 0.1f, 1.0f });
  RenderCommand::set_depth_testing(true);

  // TODO sounds here
  // ----------------

  // load textures
  // -------------

  const int tex_unit_octopus = 0;
  const int tex_unit_container = 1;

  std::cout << "loading textures... " << std::endl;
  {
    std::vector<std::pair<int, std::string>> textures_to_load;
    textures_to_load.emplace_back(tex_unit_octopus, "assets/textures/octopus.png");
    textures_to_load.emplace_back(tex_unit_container, "assets/textures/container.jpg");

    std::vector<std::thread> threads;
    std::vector<StbLoadedTexture> loaded_textures(textures_to_load.size());

    for (int i = 0; i < textures_to_load.size(); ++i) {
      const std::pair<int, std::string>& tex_to_load = textures_to_load[i];
      threads.emplace_back([&tex_to_load, i, &loaded_textures]() {
        loaded_textures[i] = load_texture(tex_to_load.second, tex_to_load.first);
      });
    }
    for (auto& thread : threads) {
      thread.join();
    }
    for (StbLoadedTexture& l : loaded_textures) {
      Texture2D tex;
      tex.generate(l);
      tex.bind(l.texture_unit);
    }
  }
  log_time_since("textures loaded ", app_start);

  // load shaders
  // ------------

  Shader solid = Shader()
                   .attach_shader("assets/shaders/lit.vert", OpenGLShaderTypes::VERTEX)
                   .attach_shader("assets/shaders/solid_colour.frag", OpenGLShaderTypes::FRAGMENT)
                   .build_program();

  // Shader basic_shader = Shader()
  //                         .attach_shader("assets/shaders/lit.vert", OpenGLShaderTypes::VERTEX)
  //                         .attach_shader("assets/shaders/basic_shader.frag", OpenGLShaderTypes::FRAGMENT)
  //                         .build_program();
  // basic_shader.bind();
  // basic_shader.set_bool("greyscale", false);
  // basic_shader.set_int("tex", tex_unit_octopus);

  // Shader shadowmap_shader =
  //   Shader()
  //     .attach_shader("assets/shaders/shadowmapping/shadow_mapping.vert", OpenGLShaderTypes::VERTEX)
  //     .attach_shader("assets/shaders/shadowmapping/shadow_mapping.frag", OpenGLShaderTypes::FRAGMENT)
  //     .build_program();
  // shadowmap_shader.bind();
  // shadowmap_shader.set_int("shadow_map", 0);

  // Shader blurShader = Shader::create("angrygl/basicer_shader.vert", "angrygl/blur_shader.frag");
  // Shader basicerShader = Shader::create("angrygl/basicer_shader.vert", "angrygl/basicer_shader.frag");
  // Shader sceneDrawShader = Shader::create("angrygl/basicer_shader.vert", "angrygl/texture_merge_shader.frag");
  // Shader simpleDepthShader = Shader::create("angrygl/depth_shader.vert", "angrygl/depth_shader.frag");
  // simpleDepthShader.use();
  // const unsigned int lsml = glGetUniformLocation(simpleDepthShader.id, "lightSpaceMatrix");
  // Shader wigglyShader = Shader::create("angrygl/wiggly_shader.vert", "angrygl/player_shader.frag");

  // Shader playerShader = Shader::create("angrygl/player_shader.vert", "angrygl/player_shader.frag");
  // playerShader.use();
  // const unsigned int playerLightSpaceMatrixLocation = glGetUniformLocation(playerShader.id, "lightSpaceMatrix");
  // playerShader.setVec3("directionLight.dir", playerLightDir);
  // playerShader.setVec3("directionLight.color", lightColor);
  // playerShader.setVec3("ambient", ambientColor);
  // playerShader.setInt("texture_spec", texUnit_playerSpec);

  log_time_since("shaders loaded ", app_start);

  // load models (note, pretty slow at the moment. could thread)
  // -----------

  Model model_1("assets/models/low_poly_knife/Knife_01.obj", false);
  Model model_2("assets/models/rpg_characters_nov_2020/OBJ/Wizard.obj", false);

  // Model player_model("angrygl/assets/Player/Player.fbx");

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

  // enemies

  // std::vector<Enemy> enemies;
  // EnemySpawner enemySpawner(monsterY, &enemies);

  // random cubes

  std::vector<glm::vec3> cube_pos;
  glm::vec3 rand_pos{ 0.0f, 0.0f, 0.0f };
  for (int i = 0; i < desired_cubes; i++) {
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

    // Settings: Shutdown app
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_END))
      app.shutdown();

    // Settings: Toggle mouse capture
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_m))
      app.get_window().toggle_mouse_capture();

    // Settings: Fullscreen
    if (app.get_input().get_key_down(SDL_KeyCode::SDLK_f)) {
      // Update window size
      app.get_window().toggle_fullscreen();
      // Update viewport size
      glm::ivec2 screen_size = app.get_window().get_size();
      RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
    }

    { // Input: Camera
      if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_W))
        camera.InputKey(delta_time_s, CameraMovement::FORWARD);
      else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_S))
        camera.InputKey(delta_time_s, CameraMovement::BACKWARD);

      if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_A))
        camera.InputKey(delta_time_s, CameraMovement::LEFT);
      else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D))
        camera.InputKey(delta_time_s, CameraMovement::RIGHT);

      if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_SPACE))
        camera.InputKey(delta_time_s, CameraMovement::UP);
      else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_LSHIFT))
        camera.InputKey(delta_time_s, CameraMovement::DOWN);
    } // ---- end camera

    // Input: Mouse
    if (app.get_window().get_mouse_captured()) {
      glm::ivec2 rel_mouse = app.get_window().get_relative_mouse_position();
      camera.InputMouse(static_cast<float>(rel_mouse.x), static_cast<float>(rel_mouse.y));
    }

    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);

    // Game State Tick
    // ---------------

    camera.Update(delta_time_s);

    // THIS IS FOR A FIXED GAME TICK
    // seconds_since_last_game_tick += delta_time_in_seconds;
    // while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
    //{
    //    //Fixed update
    //    fixed_tick(SECONDS_PER_FIXED_TICK);
    //    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
    //}

    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);

    // Rendering
    // ---------
    // renderer.update(delta_time_s, camera, rnd, cube_pos, app.get_window().get_size());

    // RenderCommand::set_viewport(0, 0, screen_size.x, screen_size.y);
    RenderCommand::clear();
    glm::mat4 view_projection = camera.get_view_projection_matrix();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
    model = glm::scale(model, glm::vec3(0.5f));

    solid.bind();
    solid.set_mat4("model", model);
    solid.set_mat4("view_projection", view_projection);
    model_2.draw();

    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);

    // GUI
    // ---

    app.gui_begin();

    // bool demo_window = true;
    // ImGui::ShowDemoWindow(&demo_window);

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
      ImGui::Text("Camera Pos: %f %f %f", camera.Position.x, camera.Position.y, camera.Position.z);
      ImGui::Text("Camera Pitch: %f", camera.Pitch);
      ImGui::Text("Camera Yaw: %f", camera.Yaw);
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

    profiler_panel.draw(app, profiler, delta_time_s);

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
// -------------------------------------------------

// Note: It'd probably be faster to do a simple line intersection from camera pos
// to plane along the direction vector.
// Probably faster too, as there's no matrix inverse else
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
