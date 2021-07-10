//
// A 2D thing.
//

// c++ lib headers
#include <iostream>
#include <vector>

// other library headers
#include <SDL2/SDL_syswm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <imgui.h>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/audio.hpp"
#include "engine/grid.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
using namespace fightingengine;

// game headers
#include "2d_game_logic.hpp"
#include "2d_game_object.hpp"
#include "2d_physics.hpp"
#include "opengl/sprite_renderer.hpp"
#include "spritemap.hpp"
using namespace game2d;

enum class GameRunning
{
  ACTIVE,
  PAUSED,
  GAME_OVER
};
GameRunning state = GameRunning::ACTIVE;

SDL_Scancode debug_key_quit = SDL_SCANCODE_ESCAPE;
SDL_Scancode debug_key_advance_one_frame = SDL_SCANCODE_RSHIFT;
SDL_Scancode debug_key_advance_one_frame_held = SDL_SCANCODE_F10;
SDL_Scancode debug_key_force_gameover = SDL_SCANCODE_F11;

bool debug_advance_one_frame = false;
bool debug_show_imgui_demo_window = false;
bool debug_render_spritesheet = true;
bool debug_show_profiler = true;

// textures
const int tex_unit_kenny_nl = 0;
const int tex_tree = 1;

// colour palette; https://colorhunt.co/palette/273312
const glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);    // black
const glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f);   // blue
const glm::vec4 PALETTE_COLOUR_3_1 = glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
const glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey

glm::vec4 background_colour = PALETTE_COLOUR_1_1;                    // black
glm::vec4 debug_line_colour = PALETTE_COLOUR_2_1;                    // blue
glm::vec4 player_colour = PALETTE_COLOUR_2_1;                        // blue
glm::vec4 bullet_colour = PALETTE_COLOUR_3_1;                        // lightblue
glm::vec4 wall_colour = PALETTE_COLOUR_4_1;                          // grey
glm::vec4 player_splat_colour = glm::vec4(0.95f, 0.3f, 0.3f, 1.0f);  // redish
glm::vec4 enemy_splat_colour = glm::vec4(0.65f, 0.65f, 0.65f, 1.0f); // greyish

sprite::type sprite_player = sprite::type::PERSON_1;
sprite::type sprite_bullet = sprite::type::WEAPON_ARROW_1;
sprite::type sprite_enemy_core = sprite::type::PERSON_2;
sprite::type sprite_weapon_base = sprite::type::WEAPON_SHOVEL;
sprite::type sprite_splat = sprite::type::CASTLE_FLOOR;

const float game_safe_radius_around_player = 7500.0f;
const float game_enemy_direct_attack_threshold = 4000.0f;
float screenshake_time = 0.1f;
float screenshake_time_left = 0.0f;

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool hide_console = false;
  if (hide_console)
    fightingengine::hide_console();

  bool ui_limit_framerate = false;
  bool ui_mute_sfx = true;
  bool ui_show_game_info = true;
  bool ui_show_entity_menu = true;
  bool ui_use_vsync = true;
  bool ui_fullscreen = false;

  glm::ivec2 screen_wh = { 1280, 720 };
  RandomState rnd;
  Application app("2D Game", screen_wh.x, screen_wh.y, ui_use_vsync);
  Profiler profiler;

  // textures

  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");
  textures_to_load.emplace_back(tex_tree, "assets/2d_game/textures/rpg/World/Bush.png");
  load_textures_threaded(textures_to_load, app_start);

  // sound

  float master_volume = 0.1f;
  audio::init_al(); // audio setup, which opens one device and one context
  // audio buffers e.g. sound effects
  ALuint audio_gunshot_0 = audio::load_sound("assets/2d_game/audio/seb/Gun_03_shoot.wav");
  ALuint audio_impact_0 = audio::load_sound("assets/2d_game/audio/seb/Impact_01.wav");
  ALuint audio_impact_1 = audio::load_sound("assets/2d_game/audio/seb/Impact_02.wav");
  ALuint audio_impact_2 = audio::load_sound("assets/2d_game/audio/seb/Impact_03.wav");
  // audio source e.g. sheep with position.
  ALuint audio_source_bullet = audio::create_source(audio_gunshot_0, master_volume / 2.0f);
  ALuint audio_source_impact_0 = audio::create_source(audio_impact_0, master_volume);
  ALuint audio_source_impact_1 = audio::create_source(audio_impact_1, master_volume);
  ALuint audio_source_impact_2 = audio::create_source(audio_impact_2, master_volume);

  log_time_since("(INFO) Audio Loaded ", app_start);

  // Rendering

  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_wh.x), static_cast<uint32_t>(screen_wh.y));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d
  sprite_renderer::init();

  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

  Shader colour_shader = Shader("2d_game/shaders/2d_basic.vert", "2d_game/shaders/2d_colour.frag");
  colour_shader.bind();

  Shader instanced_quad_shader = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
  instanced_quad_shader.bind();
  instanced_quad_shader.set_mat4("projection", projection);

  // Game

  uint32_t game_objects_destroyed = 0;

  GameObject2D tex_obj = gameobject::create_kennynl_texture(tex_unit_kenny_nl);
  GameObject2D camera = gameobject::create_camera();
  GameObject2D weapon_base;
  weapon_base.sprite = sprite_weapon_base;
  weapon_base.pos = { screen_wh.x / 2.0f, screen_wh.y / 2.0f };
  weapon_base.render_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  weapon_base.physics_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  weapon_base.collision_layer = CollisionLayer::Weapon;
  weapon_base.colour = bullet_colour;

  std::vector<GameObject2D> entities_enemies;
  std::vector<GameObject2D> entities_bullets;
  std::vector<GameObject2D> entities_player;
  std::vector<GameObject2D> entities_trees;
  std::vector<GameObject2D> entities_vfx;
  std::vector<KeysAndState> player_keys;

  int PHYSICS_GRID_SIZE = 100;
  std::vector<std::reference_wrapper<GameObject2D>> physics_grid_refs;
  int GAME_GRID_SIZE = 32;
  std::vector<std::reference_wrapper<GameObject2D>> game_grid_refs;
  std::vector<CollisionEvent> collision_events;

  // add players
  {
    GameObject2D player0 = gameobject::create_player(sprite_player, tex_unit_kenny_nl, player_colour, screen_wh);

    KeysAndState player0_keys;
    player0_keys.use_keyboard = true;

    entities_player.push_back(player0);
    player_keys.push_back(player0_keys);
  }

  std::cout << "GameObject2D is " << sizeof(GameObject2D) << " bytes" << std::endl;

  log_time_since("(INFO) End Setup ", app_start);

  while (app.is_running()) {

    Uint64 frame_start_time = SDL_GetPerformanceCounter();
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin(); // get input events
    float delta_time_s = app.get_delta_time();
    if (delta_time_s >= 0.25f)
      delta_time_s = 0.25f;

    //
    // declare a list of entities to apply all the generic things
    // e.g. physics, rendering, ui info
    //

    profiler.begin(Profiler::Stage::Physics);
    {
      if (state == GameRunning::ACTIVE || (state == GameRunning::PAUSED && debug_advance_one_frame)) {

        std::vector<std::reference_wrapper<GameObject2D>> collidable;
        collidable.insert(collidable.end(), entities_enemies.begin(), entities_enemies.end());
        collidable.insert(collidable.end(), entities_bullets.begin(), entities_bullets.end());
        collidable.insert(collidable.end(), entities_player.begin(), entities_player.end());
        collidable.insert(collidable.end(), entities_trees.begin(), entities_trees.end());
        collidable.push_back(weapon_base);

        std::vector<std::reference_wrapper<GameObject2D>> active_collidable;
        for (auto& obj : collidable) {
          if (obj.get().do_physics)
            active_collidable.push_back(obj);
        }

        // pre-physics: update grid position
        for (auto& e : active_collidable) {
          grid::get_unique_cells(e.get().pos, e.get().physics_size, PHYSICS_GRID_SIZE, e.get().in_physics_grid_cell);
        }

        // generate filtered broadphase collisions.
        std::map<uint64_t, Collision2D> filtered_collisions;
        generate_filtered_broadphase_collisions(active_collidable, filtered_collisions);

        // clear collision events this frame
        collision_events.clear();

        // Add collision to events
        for (auto& c : filtered_collisions) {
          uint32_t id_0 = c.second.ent_id_0;
          uint32_t id_1 = c.second.ent_id_1;

          // Find the objs in the read-only list
          auto& obj_0_it = std::find_if(
            collidable.begin(), collidable.end(), [&id_0](const auto& obj) { return obj.get().id == id_0; });
          auto& obj_1_it = std::find_if(
            collidable.begin(), collidable.end(), [&id_1](const auto& obj) { return obj.get().id == id_1; });

          if (obj_0_it == collidable.end() || obj_1_it == collidable.end()) {
            std::cerr << "Collision entity not in entity list" << std::endl;
            continue;
          }

          CollisionEvent eve(obj_0_it->get(), obj_1_it->get());
          collision_events.push_back(eve);
        }
      }
    }
    profiler.end(Profiler::Stage::Physics);
    profiler.begin(Profiler::Stage::SdlInput);
    {
      if (app.window_was_resized) {
        app.window_was_resized = false;

        screen_wh = app.get_window().get_size();
        RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
        glm::mat4 projection =
          glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

        instanced_quad_shader.bind();
        instanced_quad_shader.set_mat4("projection", projection);
      }

#ifdef _DEBUG

      if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE))
        app.shutdown();

      // Debug: Advance one frame
      if (app.get_input().get_key_down(debug_key_advance_one_frame)) {
        debug_advance_one_frame = true;
      }
      // Debug: Advance frames
      if (app.get_input().get_key_held(debug_key_advance_one_frame_held)) {
        debug_advance_one_frame = true;
      }
      // Debug: Start camera shake
      if (app.get_input().get_key_held(SDL_SCANCODE_COMMA)) {
        instanced_quad_shader.set_bool("shake", true);
      }
      // Debug: Stop camera shake
      if (app.get_input().get_key_held(SDL_SCANCODE_PERIOD)) {
        instanced_quad_shader.set_bool("shake", false);
      }

#endif // _DEBUG

      // glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      // printf("(game) mmb clicked %i %i \n", mouse_pos.x, mouse_pos.y);
      // glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;
    }
    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {
      { // Resolve collision events
        for (auto& event : collision_events) {

          auto& coll_layer_0 = event.go0.collision_layer;
          auto& coll_layer_1 = event.go1.collision_layer;

          if ((coll_layer_0 == CollisionLayer::Player && coll_layer_1 == CollisionLayer::Enemy) ||
              (coll_layer_1 == CollisionLayer::Player && coll_layer_0 == CollisionLayer::Enemy)) {

            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Enemy ? event.go0 : event.go1;
            GameObject2D& player = event.go0.collision_layer == CollisionLayer::Enemy ? event.go1 : event.go0;

            if (player.hits_taken >= player.hits_able_to_be_taken)
              continue; // player is dead

            enemy.flag_for_delete = true;             // enemy
            player.hits_taken += 1;                   // player
            screenshake_time_left = screenshake_time; // screenshake

            // update colour
            // float t = (player.hits_taken) / static_cast<float>(player.hits_able_to_be_taken);
            // t = glm::clamp(t, 0.0f, 1.0f); // clamp it
            // glm::vec4 col = glm::mix(player_colour, player_dead_colour, t);
            // float min_alpha = 0.7f;
            // col.a = glm::clamp(1.0f - t, min_alpha, 1.0f);
            // player.colour = col;

            // vfx spawn a splat!
            GameObject2D splat = gameobject::create_generic(sprite_splat, tex_unit_kenny_nl, player_splat_colour);
            splat.pos = player.pos;
            splat.angle_radians = fightingengine::rand_det_s(rnd.rng, 0.0f, fightingengine::PI);
            entities_vfx.push_back(splat);
          }

          if ((coll_layer_0 == CollisionLayer::Enemy && coll_layer_1 == CollisionLayer::Weapon) ||
              (coll_layer_1 == CollisionLayer::Enemy && coll_layer_0 == CollisionLayer::Weapon)) {

            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Enemy ? event.go0 : event.go1;
            GameObject2D& weapon = event.go0.collision_layer == CollisionLayer::Enemy ? event.go1 : event.go0;

            enemy.hits_taken += 1; // enemy

            // vfx for enemy-weapon collision
            GameObject2D splat = gameobject::create_generic(sprite_splat, tex_unit_kenny_nl, enemy_splat_colour);
            splat.do_lifecycle_timed = true;

            // vfx death "splat"
            if (enemy.hits_taken >= enemy.hits_able_to_be_taken) {
              splat.pos = enemy.pos;
              splat.angle_radians = fightingengine::rand_det_s(rnd.rng, 0.0f, fightingengine::PI);
              splat.time_alive_left = 30.0f;
              entities_vfx.push_back(splat);
            }

            // vfx impact "splats"
            // these splats fire off in an arc from the enemy.pos
            splat.angle_radians = 0.0f;
            splat.time_alive_left = 0.3f; // short splat
            splat.colour = { 0.0f, 1.0f, 0.0f, 1.0f };
            splat.speed_default = 40.0f;
            splat.speed_current = splat.speed_default;
            splat.physics_size = { 12.0f, 12.0f };
            splat.render_size = splat.physics_size;

            int amount_of_splats = 4;
            GameObject2D& player = entities_player[0]; // hack: use player 0 for the moment
            for (int i = 0; i < amount_of_splats; i++) {

              glm::vec2 enemy_pos_center = enemy.pos + enemy.physics_size / 2.0f;
              glm::vec2 player_pos_center = player.pos + player.physics_size / 2.0f;
              glm::vec2 distance = player_pos_center - enemy_pos_center;
              glm::vec2 dir = -glm::normalize(distance);

              glm::vec2 splat_spawn_pos = enemy.pos;
              splat_spawn_pos.x += enemy.physics_size.x / 2.0f - splat.physics_size.x / 2.0f;
              splat_spawn_pos.y += enemy.physics_size.y / 2.0f - splat.physics_size.y / 2.0f;

              splat.pos = splat_spawn_pos;

              float theta = rand_det_s(rnd.rng, -fightingengine::PI / 2.0f, fightingengine::PI / 2.0f);
              glm::vec2 offset_dir;
              offset_dir.x = cos(theta) * dir.x - sin(theta) * dir.y;
              offset_dir.y = sin(theta) * dir.x + cos(theta) * dir.y;

              splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;

              entities_vfx.push_back(splat);
            }
          }

          if ((coll_layer_0 == CollisionLayer::Obstacle && coll_layer_1 == CollisionLayer::Player) ||
              (coll_layer_1 == CollisionLayer::Obstacle && coll_layer_0 == CollisionLayer::Player)) {

            GameObject2D& obstacle = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go0 : event.go1;
            GameObject2D& player = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go1 : event.go0;

            ImGui::Begin("Huh. Well then.", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
            ImGui::Text("You are standing at a tree. Cool!");
            ImGui::End();
          }
        }
      }

      { // Update player's input
        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          KeysAndState& keys = player_keys[i];

          player::update_input(player, keys, app, camera);

          if (keys.pause_pressed)
            state = state == GameRunning::PAUSED ? GameRunning::ACTIVE : GameRunning::PAUSED;
        }
      }

      // Update game state

      if (state == GameRunning::ACTIVE) {

        // update: players

        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          KeysAndState& keys = player_keys[i];

          player::update_logic(app,
                               player,
                               keys,
                               entities_bullets,
                               tex_unit_kenny_nl,
                               bullet_colour,
                               sprite_bullet,
                               weapon_base,
                               delta_time_s);

          bool player_alive = player.invulnerable || player.hits_taken < player.hits_able_to_be_taken;
          if (!player_alive)
            state = GameRunning::GAME_OVER;
        }

        // update: bullets

        for (auto& bullet : entities_bullets) {
          bullet::update(bullet, delta_time_s);
        }

        // update: vfx

        for (auto& obj : entities_vfx) {
          gameobject::update_position(obj, delta_time_s);
        }

        // update: spawn enemies

        size_t players_in_game = entities_player.size();
        if (players_in_game > 0) {

          // for the moment, eat player 0
          GameObject2D player_to_chase = entities_player[0];

          // update with ai behaviour
          for (auto& obj : entities_enemies) {

            // check every frame: close to player?
            float distance_squared = glm::distance2(obj.pos, player_to_chase.pos);
            if (distance_squared < game_enemy_direct_attack_threshold) {
              // push new ai behaviour
              if (obj.ai_priority_list.size() > 0 && obj.ai_priority_list.back() != AiBehaviour::MOVEMENT_DIRECT) {
                obj.ai_priority_list.push_back(AiBehaviour::MOVEMENT_DIRECT);
              }
            } else {
              // far away! check if our original ai was move direct or arc angle. pop arc angle if it was pushed.
              if (obj.ai_priority_list.size() > 1 && obj.ai_priority_list.back() == AiBehaviour::MOVEMENT_DIRECT) {
                obj.ai_priority_list.pop_back();
              }
            }

            // update: ai behaviour (note, currently runs every frame probably bad)
            if (obj.ai_priority_list.size() > 0 && obj.ai_priority_list.back() == AiBehaviour::MOVEMENT_DIRECT) {
              enemy_ai::enemy_directly_to_player(obj, player_to_chase, delta_time_s);
            } else if (obj.ai_priority_list.size() > 0 &&
                       obj.ai_priority_list.back() == AiBehaviour::MOVEMENT_ARC_ANGLE) {
              enemy_ai::enemy_arc_angles_to_player(obj, player_to_chase, delta_time_s);
            }
          }

          //... and only spawn enemies if there is a player.
          enemy_spawner::update(entities_enemies,
                                camera,
                                entities_player,
                                rnd,
                                screen_wh,
                                game_safe_radius_around_player,
                                tex_unit_kenny_nl,
                                wall_colour,
                                sprite_enemy_core,
                                delta_time_s);

          // update camera pos
          camera::update(camera, player_keys[0], app, delta_time_s);
        }

        // update: screenshake
        if (screenshake_time_left > 0.0f) {
          screenshake_time_left -= delta_time_s;
          instanced_quad_shader.bind();
          instanced_quad_shader.set_bool("shake", true);
        }
        if (screenshake_time_left <= 0.0f) {
          instanced_quad_shader.bind();
          instanced_quad_shader.set_bool("shake", false);
        }

        { // object lifecycle
          gameobject::update_entities_lifecycle(entities_enemies, delta_time_s);
          gameobject::update_entities_lifecycle(entities_bullets, delta_time_s);
          gameobject::update_entities_lifecycle(entities_vfx, delta_time_s);
        }
      }
      profiler.end(Profiler::Stage::GameTick);
      profiler.begin(Profiler::Stage::Render);
      {
        RenderCommand::set_clear_colour(background_colour);
        RenderCommand::clear();
        sprite_renderer::reset_stats();
        sprite_renderer::begin_batch();
        instanced_quad_shader.bind();
        instanced_quad_shader.set_float("time", app.seconds_since_launch);

        if (state == GameRunning::ACTIVE || state == GameRunning::PAUSED || state == GameRunning::GAME_OVER) {

          std::vector<std::reference_wrapper<GameObject2D>> renderables;
          renderables.insert(renderables.end(), entities_enemies.begin(), entities_enemies.end());
          renderables.insert(renderables.end(), entities_bullets.begin(), entities_bullets.end());
          renderables.insert(renderables.end(), entities_vfx.begin(), entities_vfx.end());
          renderables.insert(renderables.end(), entities_player.begin(), entities_player.end());
          renderables.insert(renderables.end(), entities_trees.begin(), entities_trees.end());
          renderables.push_back(weapon_base);

          // all sprites from kennynl
          instanced_quad_shader.set_int("tex", tex_unit_kenny_nl);

          for (auto& obj : renderables) {
            if (!obj.get().do_render)
              continue;
            sprite_renderer::draw_sprite_debug(camera,
                                               screen_wh,
                                               instanced_quad_shader,
                                               obj.get(),
                                               obj.get().render_size,
                                               colour_shader,
                                               debug_line_colour);
          }

          if (debug_render_spritesheet) {
            // draw the spritesheet for reference
            sprite_renderer::draw_sprite_debug(
              camera, screen_wh, instanced_quad_shader, tex_obj, tex_obj.render_size, colour_shader, debug_line_colour);
          }

          sprite_renderer::end_batch();
          sprite_renderer::flush(instanced_quad_shader);
          sprite_renderer::begin_batch();

          // other sprites

          instanced_quad_shader.set_int("tex", tex_tree);

          for (auto& obj : entities_trees) {
            sprite_renderer::draw_sprite_debug(
              camera, screen_wh, instanced_quad_shader, obj, obj.render_size, colour_shader, debug_line_colour);
          }
        }

        sprite_renderer::end_batch();
        sprite_renderer::flush(instanced_quad_shader);
      }
      profiler.end(Profiler::Stage::Render);
      profiler.begin(Profiler::Stage::GuiLoop);
      {
        if (ImGui::BeginMainMenuBar()) {
          ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

          bool temp = false;

          { // limit framerate
            temp = ui_limit_framerate;
            ImGui::Checkbox("Limit Framerate", &temp);
            if (temp != ui_limit_framerate) {
              std::cout << "Limit fps toggled to: " << temp << std::endl;
              app.limit_fps = temp;
            }
            ui_limit_framerate = temp;
          }

          { // mute sfx
            temp = ui_mute_sfx;
            ImGui::Checkbox("Mute SFX", &temp);
            if (temp != ui_mute_sfx) {
              std::cout << "sfx toggled to: " << temp << std::endl;
            }
            ui_mute_sfx = temp;
          }

          { // use vsync
            temp = ui_use_vsync;
            ImGui::Checkbox("VSync", &temp);
            if (temp != ui_use_vsync) {
              std::cout << "vsync toggled to: " << temp << std::endl;
              app.get_window().set_vsync_opengl(temp);
            }
            ui_use_vsync = temp;
          }

          { // toggle fullsceren
            temp = ui_fullscreen;
            ImGui::Checkbox("Fullscreen", &ui_fullscreen);
            if (temp != ui_fullscreen) {
              std::cout << "ui_fullscreen toggled to: " << temp << std::endl;

              // hack
              app.get_window().toggle_fullscreen(); // SDL2 window toggle
              glm::ivec2 screen_wh = app.get_window().get_size();
              RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
              glm::mat4 projection =
                glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);
              instanced_quad_shader.bind();
              instanced_quad_shader.set_mat4("projection", projection);
            }
            ui_fullscreen = temp;
          }

          ImGui::SameLine(screen_wh.x - 50.0f);
          if (ImGui::MenuItem("Quit", "Esc"))
            app.shutdown();

          ImGui::EndMainMenuBar();
        }

        if (ui_show_game_info) {
          ImGui::Begin("Game Info", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
          {
            for (int i = 0; i < entities_player.size(); i++) {
              GameObject2D& player = entities_player[i];
              ImGui::Text("GO Destroyed: %i", game_objects_destroyed);
              ImGui::Text("PLAYER_ID: %i", player.id);
              ImGui::Text("PLAYER_HP_MAX %i", player.hits_able_to_be_taken);
              ImGui::Text("PLAYER_HITS_TAKEN %i", player.hits_taken);
              ImGui::Text("PLAYER_BOOST %f", player.shift_boost_time_left);
              ImGui::Text("pos %f %f", player.pos.x, player.pos.y);
              ImGui::Text("vel x: %f y: %f", player.velocity.x, player.velocity.y);
              ImGui::Text("angle %f", player.angle_radians);
              ImGui::Separator();
            }

            ImGui::Text("game running for: %f", app.seconds_since_launch);
            ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
            ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
            ImGui::Text("PhysicsGridSize %i", PHYSICS_GRID_SIZE);

            // collect number of ARC_ANGLE ai

            ImGui::Separator();
            ImGui::Text("controllers %i", SDL_NumJoysticks());
            ImGui::Separator();
            ImGui::Text("draw_calls: %i", sprite_renderer::get_draw_calls());
            ImGui::Text("quad_verts: %i", sprite_renderer::get_quad_count());
          }
          ImGui::End();
        }

        if (ui_show_entity_menu) {

          ImGui::Begin("Entity Menu", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
          {
            ImGui::Text("Players: %i", entities_player.size());
            ImGui::Text("Bullets: %i", entities_bullets.size());
            ImGui::Text("Enemies: %i", entities_enemies.size());
            ImGui::Text("Vfx: %i", entities_vfx.size());

            ImGui::Separator();

            // for (auto& e : renderables) {
            //   for (auto& c : e.get().in_physics_grid_cell) {
            //     ImGui::Text("%i E: %s x:%i y:%i ai:%i",
            //                 e.get().id,
            //                 e.get().name.c_str(),
            //                 c.x,
            //                 c.y,
            //                 e.get().ai_priority_list.size());
            //   }
            //   ImGui::Separator();
          }
        }
        ImGui::End();
      }

      if (debug_show_profiler)
        profiler_panel::draw(profiler, delta_time_s);
      if (debug_show_imgui_demo_window)
        ImGui::ShowDemoWindow(&debug_show_imgui_demo_window);
    }
    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);
    {
      debug_advance_one_frame = false;
      app.frame_end(frame_start_time);
    }
    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
}
