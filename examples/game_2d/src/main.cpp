//
// A 2D thing.
//

// c++ lib headers
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

// other library headers
#include "thirdparty/magic_enum.hpp"
#include <GL/glew.h> // temp while working out lighting
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_syswm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
// #include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <imgui.h>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/audio.hpp"
#include "engine/grid.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/util.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
using namespace fightingengine;

// game headers
#include "2d_game_config.hpp"
#include "2d_game_logic.hpp"
#include "2d_game_object.hpp"
#include "2d_physics.hpp"
#include "2d_vfx.hpp"
#include "lighting.hpp"
#include "opengl/sprite_renderer.hpp"
#include "opengl/triangle_fan.hpp"
#include "spritemap.hpp"
using namespace game2d;

enum class EditorMode
{
  EDITOR_PLACE_MODE,
  EDITOR_SELECT_MODE,
  PLAYER_ATTACK,
};
enum class GameRunning
{
  ACTIVE,
  PAUSED,
  GAME_OVER
};
enum class GamePhase
{
  ATTACK,
  SHOP,
};

glm::ivec2
toggle_fullscreen(Application& app, Shader& s)
{
  app.get_window().toggle_fullscreen(); // SDL2 window toggle
  glm::ivec2 screen_wh = app.get_window().get_size();
  RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);
  s.bind();
  s.set_mat4("projection", projection);

  return screen_wh;
}

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool hide_windows_console = true;
  if (hide_windows_console)
    fightingengine::hide_windows_console();

  SDL_Scancode debug_key_quit = SDL_SCANCODE_ESCAPE;
  SDL_Scancode debug_key_advance_one_frame = SDL_SCANCODE_RSHIFT;
  SDL_Scancode debug_key_advance_one_frame_held = SDL_SCANCODE_F10;
  SDL_Scancode debug_key_force_gameover = SDL_SCANCODE_F11;

  bool debug_advance_one_frame = false;
  bool debug_show_imgui_demo_window = false;
  bool debug_render_spritesheet = false;
  bool debug_show_profiler = true;

  bool ui_limit_framerate = false;
  bool ui_mute_sfx = true;
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
  load_textures_threaded(textures_to_load, app_start);

  unsigned int fbo_main_scene = Framebuffer::create_fbo();
  unsigned int fbo_lighting = Framebuffer::create_fbo();

  Framebuffer::bind_fbo(fbo_main_scene);
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_wh.x), static_cast<uint32_t>(screen_wh.y));
  RenderCommand::set_depth_testing(false);
  { // create a main scene texture
    unsigned int tex_id;
    glGenTextures(1, &tex_id);
    std::cout << "binding tex_unit_main_scene to " << tex_unit_main_scene << std::endl;
    glActiveTexture(GL_TEXTURE0 + tex_unit_main_scene); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_wh.x, screen_wh.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // attach it to the currently bound framebuffer object
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
      // tell opengl which colour attachments we'll use of this framebuffer
      unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
      glDrawBuffers(1, attachments);
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "(FBO: main_scene) ERROR: Framebuffer not complete!" << std::endl;
        exit(1);
      }
    }
    Framebuffer::default_fbo();
  }

  Framebuffer::bind_fbo(fbo_lighting);
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_wh.x), static_cast<uint32_t>(screen_wh.y));
  RenderCommand::set_depth_testing(false);
  { // create a lighting texture
    unsigned int tex_id;
    glGenTextures(1, &tex_id);
    std::cout << "binding tex_unit_lighting to " << tex_unit_lighting << std::endl;
    glActiveTexture(GL_TEXTURE0 + tex_unit_lighting); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_wh.x, screen_wh.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // attach it to the currently bound framebuffer object
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
      // tell opengl which colour attachments we'll use of this framebuffer
      unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
      glDrawBuffers(1, attachments);
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "(FBO: lighting) ERROR: Framebuffer not complete!" << std::endl;
        exit(1);
      }
    }
    Framebuffer::default_fbo();
  }

  // sound

  float master_volume = 0.1f;
  // audio setup, which opens one device and one context
  audio::init_al();
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
  triangle_fan_renderer::init();
  print_gpu_info();

  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

  Shader colour_shader = Shader("2d_game/shaders/2d_basic.vert", "2d_game/shaders/2d_colour.frag");
  colour_shader.bind();

  std::vector<GameObject2D> point_lights;
  {
    GameObject2D point_light;
    point_light.pos = { screen_wh.x / 2.0f, screen_wh.y / 2.0f };
    point_light.render_size = { 4.0f, 4.0f };
    point_light.physics_size = { 4.0f, 4.0f };
    point_light.sprite = sprite::type::SKULL_AND_BONES;
    point_lights.push_back(point_light);
    point_lights.push_back(point_light);
  }

  Shader fan_shader = Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");
  fan_shader.bind();
  fan_shader.set_mat4("projection", projection);

  Shader instanced_quad_shader = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
  instanced_quad_shader.bind();
  instanced_quad_shader.set_mat4("projection", projection);
  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  instanced_quad_shader.set_int_array("textures", textures, 3);

  // Game

  GameRunning state = GameRunning::ACTIVE;
  GamePhase game_phase = GamePhase::ATTACK;
  EditorMode editor_left_click_mode = EditorMode::PLAYER_ATTACK;
  const int GAME_GRID_SIZE = 32;
  const float screenshake_time = 0.1f;
  float screenshake_time_left = 0.0f;
  const float vfx_flash_time = 0.2f;
  int enemies_destroyed_this_wave = 0;
  int enemies_killed = 0;
  // weapon stats

  // shop stats
  int p0_currency = 0;
  std::map<ShopItem, shop::ShopItemState> shop = shop::shop_initial_state();
  int shop_refill_pistol_ammo = 5;
  int shop_refill_shotgun_ammo = 5;
  int shop_refill_machinegun_ammo = 5;
  // chase ai
  const float game_enemy_direct_attack_threshold = 4000.0f;

  // game objs

  game2d::MeleeWeaponStats stats_shovel(1);
  game2d::RangedWeaponStats stats_pistol(14.0f, false, 20, 3, 0.2f);
  game2d::RangedWeaponStats stats_shotgun(17.5f, false, 20, 5, 0.5f);
  game2d::RangedWeaponStats stats_machinegun(16.0f, false, 20, 4, 0.3f);

  GameObject2D tex_obj = gameobject::create_kennynl_texture();
  GameObject2D camera = GameObject2D();
  GameObject2D weapon_shovel = gameobject::create_weapon(sprite_weapon_base, tex_unit_kenny_nl, weapon_shovel_colour);
  GameObject2D weapon_pistol = gameobject::create_weapon(sprite_pistol, tex_unit_kenny_nl, weapon_pistol_colour);
  GameObject2D weapon_shotgun = gameobject::create_weapon(sprite_shotgun, tex_unit_kenny_nl, weapon_shotgun_colour);
  GameObject2D weapon_machinegun =
    gameobject::create_weapon(sprite_machinegun, tex_unit_kenny_nl, weapon_machinegun_colour);

  std::vector<Attack> attacks;
  std::vector<CollisionEvent> collision_events;
  std::vector<GameObject2D> entities_bullets;
  std::vector<GameObject2D> entities_enemies;
  std::vector<GameObject2D> entities_player;
  std::vector<GameObject2D> entities_shops;
  std::vector<GameObject2D> entities_trees;
  std::vector<GameObject2D> entities_vfx;
  std::vector<KeysAndState> player_keys;
  std::vector<std::vector<ShopItem>> player_inventories;

  // add players
  {
    GameObject2D p0 = gameobject::create_player(sprite_player, tex_unit_kenny_nl, player_colour, screen_wh);
    entities_player.push_back(p0);

    KeysAndState p0_keys;
    p0_keys.use_keyboard = true;
    player_keys.push_back(p0_keys);

    std::vector<ShopItem> p0_inventory = std::vector<ShopItem>();
    p0_inventory.push_back(ShopItem::SHOVEL);
    player_inventories.push_back(p0_inventory);
  }

  std::cout << "(INFO) GameObject2D is " << sizeof(GameObject2D) << " bytes" << std::endl;
  log_time_since("(INFO) End Setup ", app_start);

  while (app.is_running()) {

    Uint64 frame_start_time = SDL_GetPerformanceCounter();
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin(); // get input events
    float delta_time_s = app.get_delta_time();
    if (delta_time_s >= 0.25f)
      delta_time_s = 0.25f;

    profiler.begin(Profiler::Stage::Physics);
    {
      if (state == GameRunning::ACTIVE || (state == GameRunning::PAUSED && debug_advance_one_frame)) {

        std::vector<std::reference_wrapper<GameObject2D>> collidable;
        collidable.insert(collidable.end(), entities_enemies.begin(), entities_enemies.end());
        collidable.insert(collidable.end(), entities_bullets.begin(), entities_bullets.end());
        collidable.insert(collidable.end(), entities_player.begin(), entities_player.end());
        collidable.insert(collidable.end(), entities_trees.begin(), entities_trees.end());
        collidable.push_back(weapon_shovel);

        std::vector<std::reference_wrapper<GameObject2D>> active_collidable;
        for (auto& obj : collidable) {
          if (obj.get().do_physics)
            active_collidable.push_back(obj);
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

      static int i = 0;
      if (app.get_input().get_mouse_mmb_down()) {
        point_lights[1].pos = app.get_input().get_mouse_pos();
      }
      point_lights[0].pos = entities_player[0].pos;

#endif // _DEBUG

      float mousewheel = app.get_input().get_mousewheel_y();
      float epsilon = 0.0001f;
      if (mousewheel > epsilon || mousewheel < -epsilon) {
        // int wheel_int = static_cast<int>(mousewheel);
        // std::cout << "wheel int: " << wheel_int << std::endl;
        bool positive_direction = mousewheel > 0;

        // cycle through weapons for p0
        GameObject2D& p0 = entities_player[0];
        auto& p0_inventory = player_inventories[0];
        int cur_item_index = p0.equipped_item_index;
        if (positive_direction)
          cur_item_index = (cur_item_index + 1) % p0_inventory.size();
        else if (cur_item_index == 0)
          cur_item_index = static_cast<int>(p0_inventory.size() - 1);
        else
          cur_item_index = (cur_item_index - 1) % p0_inventory.size();

        p0.equipped_item_index = cur_item_index;
        // std::cout << "equipping item: " << cur_item_index << "mouse was pos: " << positive_direction << std::endl;
      }

      if (app.get_input().get_mouse_rmb_down()) {

        if (editor_left_click_mode == EditorMode::PLAYER_ATTACK)
          editor_left_click_mode = EditorMode::EDITOR_PLACE_MODE;
        else if (editor_left_click_mode == EditorMode::EDITOR_PLACE_MODE)
          editor_left_click_mode = EditorMode::EDITOR_SELECT_MODE;
        else if (editor_left_click_mode == EditorMode::EDITOR_SELECT_MODE)
          editor_left_click_mode = EditorMode::PLAYER_ATTACK;

        auto mode = std::string(magic_enum::enum_name(editor_left_click_mode));
        std::cout << "editor mode: " << mode << std::endl;
      }

      bool lmb_clicked = app.get_input().get_mouse_lmb_down();
      if (lmb_clicked && editor_left_click_mode == EditorMode::EDITOR_PLACE_MODE) {

        glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
        printf("(game) clicked gamegrid %i %i \n", mouse_pos.x, mouse_pos.y);
        glm::vec2 world_pos = glm::vec2(mouse_pos) + camera.pos;

        GameObject2D tree = gameobject::create_tree();
        tree.pos = grid::convert_world_space_to_grid_space(world_pos, GAME_GRID_SIZE);
        tree.pos = grid::convert_grid_space_to_worldspace(tree.pos, GAME_GRID_SIZE);
        tree.render_size = glm::ivec2(GAME_GRID_SIZE);
        tree.physics_size = glm::ivec2(GAME_GRID_SIZE);

        entities_trees.push_back(tree);
      }

      // Shader hot reloading
      // if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
      //   reload_shader_program(&fun_shader.ID, "2d_texture.vert", "effects/posterized_water.frag");
      //   fun_shader.bind();
      //   fun_shader.set_mat4("projection", projection);
      //   fun_shader.set_int("tex", tex_unit_kenny_nl);
      // }
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

            if (player.damage_taken >= player.damage_able_to_be_taken)
              continue; // player is dead

            enemy.flag_for_delete = true;                       // enemy
            player.damage_taken += enemy.damage_to_give_player; // player
            player.flash_time_left = vfx_flash_time;            // vfx: flash
            screenshake_time_left = screenshake_time;           // screenshake

            // vfx spawn a splat
            GameObject2D splat = gameobject::create_generic(sprite_splat, player_splat_colour);
            splat.pos = player.pos;
            splat.angle_radians = fightingengine::rand_det_s(rnd.rng, 0.0f, fightingengine::PI);
            entities_vfx.push_back(splat);
          }

          if ((coll_layer_0 == CollisionLayer::Enemy && coll_layer_1 == CollisionLayer::Weapon) ||
              (coll_layer_1 == CollisionLayer::Enemy && coll_layer_0 == CollisionLayer::Weapon)) {

            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Enemy ? event.go0 : event.go1;
            GameObject2D& weapon = event.go0.collision_layer == CollisionLayer::Enemy ? event.go1 : event.go0;
            GameObject2D& player = entities_player[0]; // hack: use player 0 for the moment

            for (auto& attack : attacks) {

              bool is_shovel = attack.weapon_type == ShopItem::SHOVEL;
              bool collision_with_specific_shovel_attack = weapon.id == attack.entity_weapon_id;
              bool taken_damage_from_shovel = std::find(enemy.attack_ids_taken_damage_from.begin(),
                                                        enemy.attack_ids_taken_damage_from.end(),
                                                        attack.id) != enemy.attack_ids_taken_damage_from.end();

              if (is_shovel && collision_with_specific_shovel_attack && !taken_damage_from_shovel) {
                // std::cout << "enemy taking damage from weapon attack ONCE!" << std::endl;
                enemy.damage_taken += attack.weapon_damage;
                enemy.attack_ids_taken_damage_from.push_back(attack.id);
                enemy.flash_time_left = vfx_flash_time; // vfx: flash

                // vfx impactsplat
                int damage_amount = attack.weapon_damage;
                vfx::spawn_impact_splats(
                  rnd, player, enemy, sprite_splat, enemy_impact_splat_colour, damage_amount, entities_vfx);
              }
            }
          }

          if ((coll_layer_0 == CollisionLayer::Bullet && coll_layer_1 == CollisionLayer::Enemy) ||
              (coll_layer_1 == CollisionLayer::Bullet && coll_layer_0 == CollisionLayer::Enemy)) {
            GameObject2D& bullet = event.go0.collision_layer == CollisionLayer::Bullet ? event.go0 : event.go1;
            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Bullet ? event.go1 : event.go0;
            GameObject2D& player = entities_player[0]; // hack: use player 0 for the moment

            for (auto& attack : attacks) {

              bool is_bullet = attack.weapon_type == ShopItem::PISTOL;
              bool collision_with_specific_bullet = bullet.id == attack.entity_weapon_id;
              bool taken_damage_from_bullet = std::find(enemy.attack_ids_taken_damage_from.begin(),
                                                        enemy.attack_ids_taken_damage_from.end(),
                                                        attack.id) != enemy.attack_ids_taken_damage_from.end();

              if (is_bullet && collision_with_specific_bullet && !taken_damage_from_bullet) {
                // std::cout << "enemy taking damage from bullet attack ONCE!" << std::endl;
                enemy.damage_taken += attack.weapon_damage;
                enemy.attack_ids_taken_damage_from.push_back(attack.id);
                enemy.flash_time_left = vfx_flash_time; // vfx: flash

                // vfx impactsplat
                int damage_amount = attack.weapon_damage;
                vfx::spawn_impact_splats(
                  rnd, player, enemy, sprite_splat, enemy_impact_splat_colour, damage_amount, entities_vfx);
              }
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

          if ((coll_layer_0 == CollisionLayer::Obstacle && coll_layer_1 == CollisionLayer::Enemy) ||
              (coll_layer_1 == CollisionLayer::Obstacle && coll_layer_0 == CollisionLayer::Enemy)) {
            GameObject2D& obstacle = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go0 : event.go1;
            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Obstacle ? event.go1 : event.go0;

            // std::cout << "enemy taking damage from bullet attack ONCE!" << std::endl;
            const int DAMAGE_TO_GIVE_ENEMY_FROM_OBSTACLE = 10;
            enemy.damage_taken += DAMAGE_TO_GIVE_ENEMY_FROM_OBSTACLE;
            enemy.flash_time_left = vfx_flash_time;

            const int DAMAGE_TO_GIVE_OBSTACLE_FROM_ENEMY = 1;
            obstacle.damage_taken += DAMAGE_TO_GIVE_OBSTACLE_FROM_ENEMY;
            obstacle.flash_time_left = vfx_flash_time;

            // vfx impactsplat
            int damage_amount = DAMAGE_TO_GIVE_ENEMY_FROM_OBSTACLE;
            vfx::spawn_impact_splats(
              rnd, enemy, obstacle, sprite_splat, enemy_impact_splat_colour, damage_amount, entities_vfx);
          }
        }
      }

      { // Update player's input
        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          KeysAndState& keys = player_keys[i];

          player::update_input(player, keys, app, camera);

          if (keys.pause_down) {
            state = state == GameRunning::PAUSED ? GameRunning::ACTIVE : GameRunning::PAUSED;
          }
        };
      }

      // Update game state

      if (state == GameRunning::ACTIVE) {

        // update: players

        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          KeysAndState& keys = player_keys[i];
          auto& player_inventory = player_inventories[i];

          player.velocity.x = keys.l_analogue_x;
          player.velocity.y = keys.l_analogue_y;
          player.velocity *= player.speed_current;

          player::ability_boost(player, keys, delta_time_s);
          gameobject::update_position(player, delta_time_s);

          if (editor_left_click_mode == EditorMode::PLAYER_ATTACK) {
            player::player_attack(app,
                                  player,
                                  stats_shovel,
                                  stats_pistol,
                                  stats_shotgun,
                                  stats_machinegun,
                                  weapon_shovel,
                                  weapon_pistol,
                                  weapon_shotgun,
                                  weapon_machinegun,
                                  player_inventory,
                                  delta_time_s,
                                  keys,
                                  attacks,
                                  entities_bullets);
          }

          bool player_alive = player.invulnerable || player.damage_taken < player.damage_able_to_be_taken;
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

        // update: vfx flash

        for (auto& obj : entities_player) {
          if (obj.flash_time_left > 0.0f) {
            obj.flash_time_left -= delta_time_s;
            obj.colour = enemy_impact_splat_colour;
          } else {
            obj.colour = player_colour;
          }
        }
        for (auto& obj : entities_enemies) {
          if (obj.flash_time_left > 0.0f) {
            obj.flash_time_left -= delta_time_s;
            obj.colour = enemy_impact_splat_colour;
          } else {
            obj.colour = enemy_colour;
          }
        }
        if (weapon_pistol.flash_time_left > 0.0f) {
          weapon_pistol.flash_time_left -= delta_time_s;
          weapon_pistol.colour = weapon_pistol_flash_colour;
        } else {
          weapon_pistol.colour = weapon_pistol_colour;
        }

        // update: vfx screenshake

        if (screenshake_time_left > 0.0f) {
          screenshake_time_left -= delta_time_s;
          instanced_quad_shader.bind();
          instanced_quad_shader.set_bool("shake", true);
        }
        if (screenshake_time_left <= 0.0f) {
          instanced_quad_shader.bind();
          instanced_quad_shader.set_bool("shake", false);
        }

        // update: vfx fade

        for (auto& go : entities_vfx) {
          go.colour.a = go.time_alive_left / go.time_alive;
        }

        // game phase: attack
        // update: spawn enemies

        size_t players_in_game = entities_player.size();
        if (players_in_game > 0 && game_phase == GamePhase::ATTACK) {

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
          enemy_spawner::update(entities_enemies, entities_player, camera, rnd, screen_wh, delta_time_s);

          // update camera pos
          camera::update(camera, player_keys[0], app, delta_time_s);
        }

        { // object lifecycle

          gameobject::update_entities_lifecycle(entities_enemies, delta_time_s);
          gameobject::update_entities_lifecycle(entities_bullets, delta_time_s);
          gameobject::update_entities_lifecycle(entities_trees, delta_time_s);
          gameobject::update_entities_lifecycle(entities_vfx, delta_time_s);

          // remove "attack" object before deleting "bullet" object (or any object that is cleaned up)
          // e.g when deleting "player" (in the future)
          std::vector<Attack>::iterator it = attacks.begin();
          while (it != attacks.end()) {
            const Attack& attack = (*it);
            int id = attack.entity_weapon_id;

            if (attack.weapon_type == ShopItem::PISTOL || attack.weapon_type == ShopItem::SHOTGUN ||
                attack.weapon_type == ShopItem::MACHINEGUN) {
              const auto& bullet = std::find_if(
                entities_bullets.begin(), entities_bullets.end(), [&id](const auto& obj) { return obj.id == id; });

              if (bullet != entities_bullets.end() && bullet->flag_for_delete) {
                // remove the attack object
                it = attacks.erase(it);
                continue;
              }
            }
            ++it;
          }

          // enemy has died
          for (auto& enemy : entities_enemies) {
            if (enemy.flag_for_delete) {
              vfx::spawn_death_splat(rnd, enemy, enemy.sprite, enemy.colour, entities_vfx);
              enemies_destroyed_this_wave += 1;
              enemies_killed += 1;
              p0_currency += 1;
            }
          }

          gameobject::erase_entities_that_are_flagged_for_delete(entities_enemies, delta_time_s);
          gameobject::erase_entities_that_are_flagged_for_delete(entities_bullets, delta_time_s);
          gameobject::erase_entities_that_are_flagged_for_delete(entities_trees, delta_time_s);
          gameobject::erase_entities_that_are_flagged_for_delete(entities_vfx, delta_time_s);
        }

        // Check if game phase is over
        {
          if (entities_enemies.size() == 0 && enemy_spawner::enemies_left_to_spawn() == 0) {
            game_phase = GamePhase::SHOP;
          }
        }
      }

      // end game-active
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    {
      //
      // lighting fbo
      //
      Framebuffer::bind_fbo(fbo_lighting);
      // Framebuffer::default_fbo();
      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      const glm::vec4 shadow_colour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      const glm::vec4 light_colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
      RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
      RenderCommand::set_depth_testing(false);
      RenderCommand::clear();
      {
        // angle, x, y for intersections
        std::vector<std::tuple<float, float, float>> intersections;

        std::vector<std::reference_wrapper<GameObject2D>> lighting_entities;
        lighting_entities.insert(lighting_entities.end(), entities_enemies.begin(), entities_enemies.end());
        lighting_entities.insert(lighting_entities.end(), entities_player.begin(), entities_player.end());
        lighting_entities.insert(lighting_entities.end(), entities_trees.begin(), entities_trees.end());
        lighting_entities.insert(lighting_entities.end(), entities_bullets.begin(), entities_bullets.end());

        std::vector<GameObject2D> lights = { point_lights[1] };
        for (auto& light : lights) {
          // for (auto& light : point_lights) {
          glm::vec2 light_pos = light.pos;

          // this generates collision from the light point to the entities
          generate_intersections(camera, light_pos, lighting_entities, screen_wh, intersections);

          // now create a triangle fan from the generated information
          triangle_fan_renderer::reset_quad_vert_count();
          triangle_fan_renderer::begin_batch();
          fan_shader.bind();
          fan_shader.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

          { // render light
            triangle_fan_renderer::add_point_to_fan(camera, fan_shader, light_pos);
            for (int i = 0; i < intersections.size(); i++) {
              auto& intersection = intersections[i];
              glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
              triangle_fan_renderer::add_point_to_fan(camera, fan_shader, pos);
            }
            auto& intersection = intersections[0];
            glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
            triangle_fan_renderer::add_point_to_fan(camera, fan_shader, pos);
            triangle_fan_renderer::end_batch();
            triangle_fan_renderer::flush(fan_shader);
          }
        }
      }
      //
      // main scene fbo
      //
      Framebuffer::bind_fbo(fbo_main_scene);
      RenderCommand::set_clear_colour(background_colour);
      RenderCommand::set_depth_testing(false);
      RenderCommand::clear();
      {
        sprite_renderer::reset_quad_vert_count();
        sprite_renderer::begin_batch();
        instanced_quad_shader.bind();
        instanced_quad_shader.set_float("time", app.seconds_since_launch);
        instanced_quad_shader.set_mat4("projection", projection);
        instanced_quad_shader.set_bool("do_lighting", false);

        if (state == GameRunning::ACTIVE || state == GameRunning::PAUSED || state == GameRunning::GAME_OVER) {
          std::vector<std::reference_wrapper<GameObject2D>> renderables;
          renderables.insert(renderables.end(), entities_vfx.begin(), entities_vfx.end());
          renderables.insert(renderables.end(), entities_enemies.begin(), entities_enemies.end());
          renderables.insert(renderables.end(), entities_bullets.begin(), entities_bullets.end());
          renderables.push_back(weapon_shovel);
          renderables.push_back(weapon_pistol);
          renderables.push_back(weapon_shotgun);
          renderables.push_back(weapon_machinegun);
          renderables.insert(renderables.end(), entities_player.begin(), entities_player.end());
          renderables.insert(renderables.end(), entities_trees.begin(), entities_trees.end());
          renderables.insert(renderables.end(), point_lights.begin(), point_lights.end());

          for (auto& obj : renderables) {
            if (!obj.get().do_render)
              continue;
            sprite_renderer::draw_instanced_sprite(camera, screen_wh, instanced_quad_shader, obj.get());
          }
          sprite_renderer::draw_sprites_debug(camera, screen_wh, renderables, colour_shader, debug_line_colour);

          if (debug_render_spritesheet) {
            sprite_renderer::draw_instanced_sprite(camera, screen_wh, instanced_quad_shader, tex_obj);
          }
        } // <!-- end GameRunning::Active -->
        sprite_renderer::end_batch();
        sprite_renderer::flush(instanced_quad_shader);
      } // <!-- end main scene fbo -->

      //
      // default fbo, draw a texture to a quad
      //
      Framebuffer::default_fbo();
      RenderCommand::set_clear_colour(background_colour);
      RenderCommand::set_depth_testing(false);
      RenderCommand::clear();
      {
        sprite_renderer::reset_quad_vert_count();
        sprite_renderer::begin_batch();
        instanced_quad_shader.bind();
        instanced_quad_shader.set_float("time", app.seconds_since_launch);
        glm::mat4 flip = glm::mat4(1.0f); // flip because opengl textures have different axis
        flip = glm::rotate(flip, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        instanced_quad_shader.set_mat4("projection", flip * projection);
        instanced_quad_shader.set_bool("do_lighting", true);
        {
          glm::vec3 pos = glm::vec3(point_lights[0].pos.x, glm::abs(point_lights[0].pos.y - screen_wh.y), 0.0f);
          instanced_quad_shader.set_vec3("light_pos[0]", glm::vec3(glm::vec2(pos.x, pos.y), 0.0f));
          pos = glm::vec3(point_lights[1].pos.x, glm::abs(point_lights[1].pos.y - screen_wh.y), 0.0f);
          instanced_quad_shader.set_vec3("light_pos[1]", glm::vec3(glm::vec2(pos.x, pos.y), 0.0f));
        }

        { // draw single quad as entire screen
          GameObject2D screen_object = gameobject::create_generic(sprite::type::EMPTY, glm::vec4(1.0f));
          screen_object.render_size = glm::vec2(screen_wh.x, screen_wh.y);
          screen_object.tex_slot = tex_unit_main_scene;
          sprite_renderer::draw_instanced_sprite(camera, screen_wh, instanced_quad_shader, screen_object);
        }
        sprite_renderer::end_batch();
        sprite_renderer::flush(instanced_quad_shader);
      }
    }
    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    {
      // ui: shop window

      if (game_phase == GamePhase::SHOP) {
        ImGui::Begin("Humble Wares", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

        update_shop(p0_currency, shop, player_inventories, entities_player);

        if (ImGui::Button("Leave the shop, and never return! Or will you?")) {
          std::cout << "clicked leave shop" << std::endl;
          enemy_spawner::next_wave();
          game_phase = GamePhase::ATTACK;
        }
        ImGui::End();
      }

      // ui: top menu bar

      if (ImGui::BeginMainMenuBar()) {

        ImGui::Text("WAVE %i", enemy_spawner::get_wave());

        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          auto& p_inventory = player_inventories[i];

          ImGui::Text("(P%i) HP %i", i, player.damage_able_to_be_taken - player.damage_taken);
          ImGui::Text("BOOST %.2fs", player.shift_boost_time_left);

          std::string ammo_pistol_label = "AMMO_PISTOL " + std::to_string(stats_pistol.current_ammo);
          std::string ammo_shotgun_label = "AMMO_SHOTGUN " + std::to_string(stats_shotgun.current_ammo);
          std::string ammo_machinegun_label = "AMMO_MACHINEGUN " + std::to_string(stats_machinegun.current_ammo);
          ImGui::Text(ammo_pistol_label.c_str());
          ImGui::Text(ammo_shotgun_label.c_str());
          ImGui::Text(ammo_machinegun_label.c_str());

          ShopItem w = p_inventory[i];
          std::string wep = std::string("Weapon: ") + std::string(magic_enum::enum_name(w));
          ImGui::Text(wep.c_str());

          ImGui::SameLine(screen_wh.x - 280.0f);

          float framerate = ImGui::GetIO().Framerate;
          float framerate_ms = 1000.0f / ImGui::GetIO().Framerate;
          std::stringstream stream;
          stream << std::fixed << std::setprecision(2) << framerate;
          std::string framerate_str = stream.str();
          stream.str(std::string());
          stream << std::fixed << std::setprecision(2) << framerate;
          std::string framerate_ms_str = stream.str();
          std::string framerate_label = framerate_str + std::string(" FPS (") + framerate_ms_str + std::string(" ms)");
          ImGui::Text(framerate_label.c_str());

          if (ImGui::BeginMenu("Settings")) {

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
              ImGui::Checkbox("Fullscreen", &temp);
              if (temp != ui_fullscreen) {
                std::cout << "ui_fullscreen toggled to: " << temp << std::endl;
                screen_wh = toggle_fullscreen(app, instanced_quad_shader);
              }
              ui_fullscreen = temp;
            }

            { // restart button
              if (ImGui::Button("Restart Game")) {
                std::cout << "restart game" << std::endl;
              }
            }

            ImGui::EndMenu();
          }
        }

        if (ImGui::MenuItem("Quit", "Esc"))
          app.shutdown();

        ImGui::EndMainMenuBar();
      }

      // ui: profiler

      if (debug_show_profiler) {
        ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        ImGui::Text("Players: %i", entities_player.size());
        ImGui::Text("Bullets: %i", entities_bullets.size());
        ImGui::Text("Enemies: %i", entities_enemies.size());
        ImGui::Text("Trees: %i", entities_trees.size());
        ImGui::Text("Vfx: %i", entities_vfx.size());
        ImGui::Text("Attacks: %i", attacks.size());
        ImGui::Text("controllers %i", SDL_NumJoysticks());
        ImGui::Separator();
        ImGui::Text("draw_calls: %i", sprite_renderer::get_draw_calls());
        profiler_panel::draw_timers(profiler, delta_time_s);
        ImGui::End();
      }

      // ui: imgui demo window

      if (debug_show_imgui_demo_window)
        ImGui::ShowDemoWindow(&debug_show_imgui_demo_window);
    }
    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);
    {
      debug_advance_one_frame = false;
      sprite_renderer::end_frame();
      app.frame_end(frame_start_time);
    }
    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
}
