//
// A 2D thing.
//

// c++ lib headers
#include <iostream>
#include <vector>

// other library headers
#include "thirdparty/magic_enum.hpp"
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
#include "2d_game_config.hpp"
#include "2d_game_logic.hpp"
#include "2d_game_object.hpp"
#include "2d_physics.hpp"
#include "2d_vfx.hpp"
#include "opengl/sprite_renderer.hpp"
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

int
main()
{
  std::cout << "booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool hide_console = true;
#ifndef _DEBUG
  if (hide_console)
    fightingengine::hide_console();
#endif

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
  bool ui_show_game_info = true;
  bool ui_show_entity_menu = true;
  bool ui_use_vsync = true;
  bool ui_fullscreen = false;

  glm::ivec2 screen_wh = { 1280, 720 };
  RandomState rnd;
  Application app("2D Game", screen_wh.x, screen_wh.y, ui_use_vsync);
  Profiler profiler;

  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");
  textures_to_load.emplace_back(tex_unit_tree, "assets/2d_game/textures/rpg/foliage_bush.png");
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
  int textures[2] = { tex_unit_kenny_nl, tex_unit_tree };
  instanced_quad_shader.set_int_array("textures", textures);

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
  game2d::RangedWeaponStats stats_pistol(14.0f, false, 20, 10, 0.2f);
  game2d::RangedWeaponStats stats_shotgun(17.5f, false, 20, 8, 0.5f);
  game2d::RangedWeaponStats stats_machinegun(16.0f, false, 20, 5, 0.3f);
  // shop stats
  int p0_currency = 0;
  int shop_refill_pistol_ammo = 5;
  int shop_refill_shotgun_ammo = 5;
  int shop_refill_machinegun_ammo = 5;
  // chase ai
  const float game_enemy_direct_attack_threshold = 4000.0f;

  // game objs

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
  std::map<ShopItem, shop::ShopItemState> shop = shop::shop_initial_state();

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

            enemy.flag_for_delete = true;             // enemy
            player.damage_taken += 1;                 // player
            player.flash_time_left = vfx_flash_time;  // vfx: flash
            screenshake_time_left = screenshake_time; // screenshake

            // vfx spawn a splat
            GameObject2D splat = gameobject::create_generic(sprite_splat, tex_unit_kenny_nl, player_splat_colour);
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
                enemy.damage_taken += 1;
                enemy.attack_ids_taken_damage_from.push_back(attack.id);
                enemy.flash_time_left = vfx_flash_time; // vfx: flash

                // vfx impactsplat
                int damage_amount = 3;
                vfx::spawn_impact_splats(rnd,
                                         player,
                                         enemy,
                                         tex_unit_kenny_nl,
                                         sprite_splat,
                                         enemy_impact_splat_colour,
                                         damage_amount,
                                         entities_vfx);
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
                enemy.damage_taken += 1;
                enemy.attack_ids_taken_damage_from.push_back(attack.id);
                enemy.flash_time_left = vfx_flash_time; // vfx: flash

                // vfx impactsplat
                int damage_amount = 3;
                vfx::spawn_impact_splats(rnd,
                                         player,
                                         enemy,
                                         tex_unit_kenny_nl,
                                         sprite_splat,
                                         enemy_impact_splat_colour,
                                         damage_amount,
                                         entities_vfx);
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
            enemy.damage_taken += 1;
            enemy.flash_time_left = vfx_flash_time;

            obstacle.damage_taken += 1;
            obstacle.flash_time_left = vfx_flash_time;

            // vfx impactsplat
            int damage_amount = 3;
            vfx::spawn_impact_splats(rnd,
                                     enemy,
                                     obstacle,
                                     tex_unit_kenny_nl,
                                     sprite_splat,
                                     enemy_impact_splat_colour,
                                     damage_amount,
                                     entities_vfx);
          }
        }
      }

      { // Update player's input
        for (int i = 0; i < entities_player.size(); i++) {
          GameObject2D& player = entities_player[i];
          KeysAndState& keys = player_keys[i];

          player::update_input(player, keys, app, camera);

          if (keys.pause_down)
            state = state == GameRunning::PAUSED ? GameRunning::ACTIVE : GameRunning::PAUSED;
        }
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

            weapon_shovel.do_render = false;
            weapon_pistol.do_render = false;
            weapon_shotgun.do_render = false;
            weapon_machinegun.do_render = false;

            if (player_inventory[player.equipped_item_index] == ShopItem::SHOVEL) {
              weapon_shovel.do_render = true;
              player::ability_slash(app, player, keys, weapon_shovel, delta_time_s, attacks);
            }

            if (player_inventory[player.equipped_item_index] == ShopItem::PISTOL) {
              weapon_pistol.do_render = true;
              float angle_around_player = keys.angle_around_player;
              glm::vec2 offset = glm::vec2(stats_pistol.radius_offset_from_player * sin(angle_around_player),
                                           -stats_pistol.radius_offset_from_player * cos(angle_around_player));
              weapon_pistol.pos = player.pos + offset;
              weapon_pistol.angle_radians =
                keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(weapon_pistol.sprite);

              if (stats_pistol.infinite_ammo || stats_pistol.current_ammo > 0) {
                player::ability_shoot(app,
                                      weapon_pistol,
                                      stats_pistol.current_ammo,
                                      keys,
                                      entities_bullets,
                                      tex_unit_kenny_nl,
                                      bullet_pistol_colour,
                                      sprite_bullet,
                                      delta_time_s,
                                      attacks);
              }
            }

            if (player_inventory[player.equipped_item_index] == ShopItem::SHOTGUN) {
              weapon_shotgun.do_render = true;
              float angle_around_player = keys.angle_around_player;
              glm::vec2 offset = glm::vec2(stats_shotgun.radius_offset_from_player * sin(angle_around_player),
                                           -stats_shotgun.radius_offset_from_player * cos(angle_around_player));
              weapon_shotgun.pos = player.pos + offset;
              weapon_shotgun.angle_radians =
                keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(weapon_shotgun.sprite);

              if (stats_shotgun.infinite_ammo || stats_shotgun.current_ammo > 0) {
                player::ability_shoot(app,
                                      weapon_shotgun,
                                      stats_shotgun.current_ammo,
                                      keys,
                                      entities_bullets,
                                      tex_unit_kenny_nl,
                                      bullet_shotgun_colour,
                                      sprite_bullet,
                                      delta_time_s,
                                      attacks);
              }
            }

            if (player_inventory[player.equipped_item_index] == ShopItem::MACHINEGUN) {
              weapon_machinegun.do_render = true;
              float angle_around_player = keys.angle_around_player;
              glm::vec2 offset = glm::vec2(stats_machinegun.radius_offset_from_player * sin(angle_around_player),
                                           -stats_machinegun.radius_offset_from_player * cos(angle_around_player));
              weapon_machinegun.pos = player.pos + offset;
              weapon_machinegun.angle_radians =
                keys.angle_around_player + sprite::spritemap::get_sprite_rotation_offset(weapon_machinegun.sprite);

              if (stats_machinegun.infinite_ammo || stats_machinegun.current_ammo > 0) {
                player::ability_shoot(app,
                                      weapon_machinegun,
                                      stats_machinegun.current_ammo,
                                      keys,
                                      entities_bullets,
                                      tex_unit_kenny_nl,
                                      bullet_machinegun_colour,
                                      sprite_bullet,
                                      delta_time_s,
                                      attacks);
              }
            }
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

        // game phase: shop

        if (game_phase == GamePhase::SHOP) {
          ImGui::Begin("Humble Wares", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
          ImGui::Text("You have %i coin!", p0_currency);

          if (ImGui::Button("Drain your coin..."))
            p0_currency -= 1;

          for (auto& shop_item : shop) {

            std::string wep = std::string(magic_enum::enum_name(shop_item.first));

            bool able_to_buy = p0_currency >= shop_item.second.price && shop_item.second.quantity > 0;
            if (able_to_buy) {
              std::string buy_button_label = "Buy ##" + wep;
              bool buy_button_clicked = ImGui::Button(buy_button_label.c_str());
              if (buy_button_clicked) {
                std::cout << "buy: " << wep << " clicked" << std::endl;

                // reduce item quantity if not infinite
                if (!shop_item.second.infinite_quantity)
                  shop_item.second.quantity -= 1;

                // spend hard earned cash
                p0_currency -= shop_item.second.price;

                // shop logic
                {
                  if (shop_item.first == ShopItem::PISTOL || shop_item.first == ShopItem::SHOTGUN ||
                      shop_item.first == ShopItem::MACHINEGUN) {
                    // hack: use player 0 for the moment
                    std::vector<ShopItem>& player_inv = player_inventories[0];
                    player_inv.push_back(shop_item.first);
                  }

                  if (shop_item.first == ShopItem::PISTOL_AMMO)
                    stats_pistol.current_ammo += shop_refill_pistol_ammo;

                  if (shop_item.first == ShopItem::SHOTGUN_AMMO)
                    stats_shotgun.current_ammo += shop_refill_shotgun_ammo;

                  if (shop_item.first == ShopItem::MACHINEGUN_AMMO)
                    stats_machinegun.current_ammo += shop_refill_machinegun_ammo;

                  if (shop_item.first == ShopItem::HEAL_HALF) {
                    GameObject2D& p0 = entities_player[0];
                    p0.damage_taken -= static_cast<int>(p0.damage_able_to_be_taken / 2);
                    if (p0.damage_taken < 0)
                      p0.damage_taken = 0;
                  }

                  if (shop_item.first == ShopItem::HEAL_FULL) {
                    GameObject2D& p0 = entities_player[0];
                    p0.damage_taken = 0;
                  }
                }
              }
              ImGui::SameLine();
            }

            ImGui::Text(
              "Item: %s Quantiy: %i Price: %i", wep.c_str(), shop_item.second.quantity, shop_item.second.price);
          }
          if (ImGui::Button("Leave the shop, and never return! Or will you?")) {
            std::cout << "clicked leave shop" << std::endl;
            enemy_spawner::next_wave();
            game_phase = GamePhase::ATTACK;
          }
          ImGui::End();
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
              vfx::spawn_death_splat(rnd, enemy, enemy.sprite, tex_unit_kenny_nl, enemy.colour, entities_vfx);
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
          renderables.insert(renderables.end(), entities_vfx.begin(), entities_vfx.end());
          renderables.insert(renderables.end(), entities_enemies.begin(), entities_enemies.end());
          renderables.insert(renderables.end(), entities_bullets.begin(), entities_bullets.end());
          renderables.push_back(weapon_shovel);
          renderables.push_back(weapon_pistol);
          renderables.push_back(weapon_shotgun);
          renderables.push_back(weapon_machinegun);
          renderables.insert(renderables.end(), entities_player.begin(), entities_player.end());
          renderables.insert(renderables.end(), entities_trees.begin(), entities_trees.end());

          if (ui_show_entity_menu) {
            ImGui::Begin("Entity Menu", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
            {
              ImGui::Text("Players: %i", entities_player.size());
              ImGui::Text("Bullets: %i", entities_bullets.size());
              ImGui::Text("Enemies: %i", entities_enemies.size());
              ImGui::Text("Vfx: %i", entities_vfx.size());
              ImGui::Text("Attacks: %i", attacks.size());
              ImGui::Separator();

              for (auto& e : renderables) {
                for (auto& c : e.get().in_physics_grid_cell) {
                  ImGui::Text("id: %i entity: %s in cell: x:%i y:%i", e.get().id, e.get().name.c_str(), c.x, c.y);
                }
              }
            }
            ImGui::End();
          }

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
              auto& p_inventory = player_inventories[i];
              ImGui::Text("PLAYER_ID: %i", player.id);
              ImGui::Text("PLAYER_HP_MAX %i", player.damage_able_to_be_taken);
              ImGui::Text("PLAYER_HITS_TAKEN %i", player.damage_taken);
              ImGui::Text("PLAYER_BOOST %f", player.shift_boost_time_left);
              ImGui::Text("AMMO_PISTOL %i", stats_pistol.current_ammo);
              ImGui::Text("AMMO_SHOTGUN %i", stats_shotgun.current_ammo);
              ImGui::Text("AMMO_MACHINEGUN %i", stats_machinegun.current_ammo);
              ImGui::Text("pos %f %f", player.pos.x, player.pos.y);
              ImGui::Text("vel x: %f y: %f", player.velocity.x, player.velocity.y);
              ImGui::Text("angle %f", player.angle_radians);
              ImGui::Separator();
              ShopItem w = p_inventory[i];
              std::string wep = std::string(magic_enum::enum_name(w));
              std::string label = std::string("Weapon: ") + wep;
              if (player.equipped_item_index == i) {
                ImGui::Text("(EQUIPPED) %s", label.c_str());
              } else {
                ImGui::Text("%s", label.c_str());
              }
              ImGui::Separator();
            }

            ImGui::Text("game running for: %f", app.seconds_since_launch);
            ImGui::Text("camera pos %f %f", camera.pos.x, camera.pos.y);
            ImGui::Text("mouse pos %f %f", app.get_input().get_mouse_pos().x, app.get_input().get_mouse_pos().y);
            ImGui::Text("WAVE %i", enemy_spawner::get_wave());

            ImGui::Separator();
            ImGui::Text("controllers %i", SDL_NumJoysticks());
            ImGui::Separator();
            ImGui::Text("draw_calls: %i", sprite_renderer::get_draw_calls());
            ImGui::Text("quad_verts: %i", sprite_renderer::get_quad_count());
          }
          ImGui::End();
        }
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
