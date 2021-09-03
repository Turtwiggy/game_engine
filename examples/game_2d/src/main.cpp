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
#include "engine/grid.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
using namespace fightingengine;

// game headers
#include "2d_game_object.hpp"
#include "2d_game_state.hpp"
#include "2d_lighting.hpp"
#include "2d_physics.hpp"
#include "2d_vfx.hpp"
#include "ai.hpp"
#include "components/resources.hpp"
#include "components/screen_shake_timer.hpp"
#include "constants.hpp"
#include "enemy_spawner.hpp"
#include "game.hpp"
#include "opengl/sprite_renderer.hpp"
#include "opengl/triangle_fan.hpp"
#include "player.hpp"
#include "spritemap.hpp"
using namespace game2d;

void
toggle_fullscreen(Application& app,
                  glm::ivec2& screen_wh,
                  glm::mat4& projection,
                  unsigned int tex_id_lighting,
                  unsigned int tex_id_main_scene)
{
  app.get_window().toggle_fullscreen(); // SDL2 window toggle
  screen_wh = app.get_window().get_size();
  RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
  projection = glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

  update_texture_size(screen_wh, tex_id_lighting);
  update_texture_size(screen_wh, tex_id_main_scene);
}

void
add_players(MutableGameState& state, glm::ivec2 screen_wh)
{
  GameObject2D p0 = gameobject::create_player(sprite_player, tex_unit_kenny_nl, player_colour, screen_wh);
  state.entities_player.push_back(p0);

  KeysAndState p0_keys;
  p0_keys.use_keyboard = true;
  state.player_keys.push_back(p0_keys);

  std::vector<ShopItem> p0_inventory = std::vector<ShopItem>();
  // p0_inventory.push_back(ShopItem::SHOVEL);
  p0_inventory.push_back(ShopItem::PISTOL);
  state.player_inventories.push_back(p0_inventory);
}

void
add_lighting(MutableGameState& state, glm::ivec2 screen_wh)
{
  SceneicPointLight point_light_0;
  point_light_0.light_object = gameobject::create_light();
  // point_light_0.light_object.pos = { int(screen_wh.x / 2.0f), int(screen_wh.y / 2.0f) };
  point_light_0.light_object.pos = { int(0), int(0) };

  SceneicPointLight point_light_1;
  point_light_1.light_object = gameobject::create_light();
  point_light_1.light_object.pos = { int(screen_wh.x), int(0) };

  SceneicPointLight point_light_2;
  point_light_2.light_object = gameobject::create_light();
  point_light_2.light_object.pos = { int(0), int(screen_wh.y) };

  SceneicPointLight point_light_3;
  point_light_3.light_object = gameobject::create_light();
  point_light_3.light_object.pos = { int(screen_wh.x), int(screen_wh.y) };

  SceneicPointLight point_light_4;
  point_light_4.light_object = gameobject::create_light();
  point_light_4.light_object.pos = { int(screen_wh.x / 2.0f), int(screen_wh.y / 2.0f) };

  state.point_lights.push_back(point_light_0);
  state.point_lights.push_back(point_light_1);
  state.point_lights.push_back(point_light_2);
  state.point_lights.push_back(point_light_3);
  state.point_lights.push_back(point_light_4);
}

MutableGameState
reset_game(glm::ivec2 screen_wh)
{
  MutableGameState default_state;
  add_players(default_state, screen_wh);
  add_lighting(default_state, screen_wh);
  return default_state;
}

int
main()
{
  std::cout << "main init..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool hide_windows_console = false;
  if (hide_windows_console)
    fightingengine::hide_windows_console();

  SDL_Scancode debug_key_advance_one_frame = SDL_SCANCODE_RSHIFT;
  SDL_Scancode debug_key_advance_one_frame_held = SDL_SCANCODE_F10;
  SDL_Scancode debug_key_force_gameover = SDL_SCANCODE_F11;

  bool debug_advance_one_frame = false;
  bool debug_show_imgui_demo_window = false;
  bool debug_show_spritesheet_window = false;
  bool debug_show_profiler = false;

  bool ui_limit_framerate = false;
  bool ui_mute_sfx = true;
  bool ui_use_vsync = true;
  bool ui_fullscreen = false;
  bool ui_esc_menu = false;

  glm::ivec2 screen_wh = { 1366, 720 };
  RandomState rnd;
  Application app("2D Game", screen_wh.x, screen_wh.y, ui_use_vsync);
  Profiler profiler;

  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_wh.x), static_cast<uint32_t>(screen_wh.y));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d
  sprite_renderer::init();
  triangle_fan_renderer::init();
  print_gpu_info();

  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl,
                                "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");

  entt::registry registry;
  game::init(registry, screen_wh, app_start);
  auto& resources = registry.ctx<Resources>();

  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  resources.instanced_quad_shader.bind();
  resources.instanced_quad_shader.set_int_array("textures", textures, 3);

  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);

  MutableGameState gs = reset_game(screen_wh);

  std::cout << "(INFO) GameObject2D is " << sizeof(GameObject2D) << " bytes" << std::endl;
  std::cout << "(INFO) MutableGameState is " << sizeof(MutableGameState) << " bytes" << std::endl;
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
      if (gs.game_running == GameRunning::ACTIVE ||
          (gs.game_running == GameRunning::PAUSED && debug_advance_one_frame)) {

        std::vector<std::reference_wrapper<GameObject2D>> collidable;
        collidable.insert(collidable.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
        collidable.insert(collidable.end(), gs.entities_bullets.begin(), gs.entities_bullets.end());
        collidable.insert(collidable.end(), gs.entities_player.begin(), gs.entities_player.end());
        collidable.insert(collidable.end(), gs.entities_trees.begin(), gs.entities_trees.end());
        collidable.push_back(gs.weapon_shovel);

        std::vector<std::reference_wrapper<GameObject2D>> active_collidable;
        for (auto& obj : collidable) {
          if (obj.get().do_physics)
            active_collidable.push_back(obj);
        }

        // generate filtered broadphase collisions.
        std::map<uint64_t, Collision2D> filtered_collisions;
        generate_filtered_broadphase_collisions(active_collidable, filtered_collisions);

        // clear collision events this frame
        gs.collision_events.clear();

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
          gs.collision_events.push_back(eve);
        }
      }
    }
    profiler.end(Profiler::Stage::Physics);
    profiler.begin(Profiler::Stage::SdlInput);
    {

#ifdef _DEBUG

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
        resources.instanced_quad_shader.bind();
        resources.instanced_quad_shader.set_bool("shake", true);
      }
      // Debug: Stop camera shake
      if (app.get_input().get_key_held(SDL_SCANCODE_PERIOD)) {
        resources.instanced_quad_shader.bind();
        resources.instanced_quad_shader.set_bool("shake", false);
      }

      if (app.get_input().get_key_down(SDL_SCANCODE_RETURN)) {
        gs.entities_enemies.clear();
        gs.enemies_to_spawn_this_wave_left = 0;
        gs.p0_currency += 100;
      }

      // Shader hot reloading
      if (app.get_input().get_key_down(SDL_SCANCODE_R)) {
        reload_shader_program(&resources.instanced_quad_shader.ID,
                              "2d_game/shaders/2d_instanced.vert",
                              "2d_game/shaders/2d_instanced.frag");
        resources.instanced_quad_shader.bind();
        resources.instanced_quad_shader.set_mat4("projection", projection);
        resources.instanced_quad_shader.set_int_array("textures", textures, 3);
      }

#endif // _DEBUG

      GameObject2D& p0 = gs.entities_player[0];
      auto& p0_inventory = gs.player_inventories[0];
      player::scroll_to_swap_weapons(app, p0, p0_inventory);

      // TODO: GRID STUFF
      // if (app.get_input().get_mouse_rmb_down()) {
      //   if (gs.editor_left_click_mode == EditorMode::PLAYER_ATTACK)
      //     gs.editor_left_click_mode = EditorMode::EDITOR_PLACE_MODE;
      //   else if (gs.editor_left_click_mode == EditorMode::EDITOR_PLACE_MODE)
      //     gs.editor_left_click_mode = EditorMode::EDITOR_SELECT_MODE;
      //   else if (gs.editor_left_click_mode == EditorMode::EDITOR_SELECT_MODE)
      //     gs.editor_left_click_mode = EditorMode::PLAYER_ATTACK;
      //   auto mode = std::string(magic_enum::enum_name(gs.editor_left_click_mode));
      //   std::cout << "editor mode: " << mode << std::endl;
      // }
      // bool lmb_clicked = app.get_input().get_mouse_lmb_down();
      // if (lmb_clicked && gs.editor_left_click_mode == EditorMode::EDITOR_PLACE_MODE) {
      //   glm::ivec2 mouse_pos = app.get_input().get_mouse_pos();
      //   printf("(game) clicked gamegrid %i %i \n", mouse_pos.x, mouse_pos.y);
      //   glm::ivec2 world_pos = mouse_pos + gs.camera.pos;
      //   GameObject2D tree = gameobject::create_tree();
      //   tree.pos = grid::convert_world_space_to_grid_space(world_pos, GAME_GRID_SIZE);
      //   tree.pos = grid::convert_grid_space_to_worldspace(tree.pos, GAME_GRID_SIZE);
      //   tree.render_size = glm::ivec2(GAME_GRID_SIZE);
      //   tree.physics_size = glm::ivec2(GAME_GRID_SIZE);
      //   gs.entities_trees.push_back(tree);
      // }

      { // Pause menu
        if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE)) {
          if (ui_esc_menu) {
            std::cout << "unpausing game" << std::endl;
            gs.game_running = GameRunning::ACTIVE;
            ui_esc_menu = false;
          } else {
            std::cout << "pause game and showing menu" << std::endl;
            gs.game_running = GameRunning::PAUSED;
            ui_esc_menu = true;
          }
        }
      }
    }
    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {
      game::update_sim(registry, delta_time_s);
      game::update(registry, delta_time_s);

      { // Resolve collision events

        for (auto& event : gs.collision_events) {

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
            auto& ss_timer = registry.ctx<ScreenShakeTimer>();
            ss_timer.time_left = screenshake_time; // screenshake

            // vfx spawn a splat
            GameObject2D splat = gameobject::create_generic();
            splat.sprite = sprite_splat;
            splat.colour = player_splat_colour;
            splat.pos = player.pos;
            splat.angle_radians = fightingengine::rand_det_s(rnd.rng, 0.0f, fightingengine::PI);
            gs.entities_vfx.push_back(splat);
          }

          if ((coll_layer_0 == CollisionLayer::Enemy && coll_layer_1 == CollisionLayer::Weapon) ||
              (coll_layer_1 == CollisionLayer::Enemy && coll_layer_0 == CollisionLayer::Weapon)) {

            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Enemy ? event.go0 : event.go1;
            GameObject2D& weapon = event.go0.collision_layer == CollisionLayer::Enemy ? event.go1 : event.go0;
            GameObject2D& player = gs.entities_player[0]; // hack: use player 0 for the moment

            for (auto& attack : gs.attacks) {

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
                vfx::spawn_impact_splats(rnd, player, enemy, sprite_splat, damage_amount, gs.entities_vfx);
              }
            }
          }

          if ((coll_layer_0 == CollisionLayer::Bullet && coll_layer_1 == CollisionLayer::Enemy) ||
              (coll_layer_1 == CollisionLayer::Bullet && coll_layer_0 == CollisionLayer::Enemy)) {
            GameObject2D& bullet = event.go0.collision_layer == CollisionLayer::Bullet ? event.go0 : event.go1;
            GameObject2D& enemy = event.go0.collision_layer == CollisionLayer::Bullet ? event.go1 : event.go0;
            GameObject2D& player = gs.entities_player[0]; // hack: use player 0 for the moment

            for (auto& attack : gs.attacks) {

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
                vfx::spawn_impact_splats(rnd, player, enemy, sprite_splat, damage_amount, gs.entities_vfx);
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
            vfx::spawn_impact_splats(rnd, enemy, obstacle, sprite_splat, damage_amount, gs.entities_vfx);
          }
        }
      }

      // update: player input

      for (int i = 0; i < gs.entities_player.size(); i++) {
        GameObject2D& player = gs.entities_player[i];
        KeysAndState& keys = gs.player_keys[i];
        auto& player_inventory = gs.player_inventories[i];
        player::update_input(player, keys, app, gs.camera);
        player.velocity.x = keys.l_analogue_x;
        player.velocity.y = keys.l_analogue_y;
        player.velocity *= player.speed_current;
      }

      // update: game state

      if (gs.game_running == GameRunning::ACTIVE) {

        // update: players

        for (int i = 0; i < gs.entities_player.size(); i++) {
          GameObject2D& player = gs.entities_player[i];
          KeysAndState& keys = gs.player_keys[i];
          auto& player_inventory = gs.player_inventories[i];
          player::ability_boost(player, keys, delta_time_s);
        }

        // update: enemies

        size_t players_in_game = gs.entities_player.size();
        if (players_in_game > 0 && gs.game_phase == GamePhase::ATTACK) {

          // for the moment, eat player 0
          GameObject2D player_to_chase = gs.entities_player[0];

          // update with ai behaviour
          for (auto& obj : gs.entities_enemies) {

            // check every frame: close to player?
            float distance_squared = glm::distance2(glm::vec2(obj.pos), glm::vec2(player_to_chase.pos));
            if (distance_squared < ENEMY_ATTACK_THRESHOLD) {
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
          enemy_spawner::update(gs, rnd, screen_wh, delta_time_s);
        }

        // look in velocity direction
        // float angle = atan2(obj.velocity.y, obj.velocity.x);
        // angle += fightingengine::HALF_PI + sprite::spritemap::get_sprite_rotation_offset(obj.sprite);

        // position update order
        {
          std::vector<std::reference_wrapper<GameObject2D>> update_positions;
          update_positions.insert(update_positions.end(), gs.entities_player.begin(), gs.entities_player.end());
          update_positions.insert(update_positions.end(), gs.entities_vfx.begin(), gs.entities_vfx.end());
          update_positions.insert(update_positions.end(), gs.entities_bullets.begin(), gs.entities_bullets.end());
          update_positions.insert(update_positions.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());

          for (auto& ent : update_positions) {
            gameobject::update_position_x(ent, delta_time_s);
            gameobject::update_position_y(ent, delta_time_s);
          }
        }

        for (int i = 0; i < gs.entities_player.size(); i++) {
          GameObject2D& player = gs.entities_player[i];
          KeysAndState& keys = gs.player_keys[i];
          auto& player_inventory = gs.player_inventories[i];

          // clamp players to screen
          if (player.pos.x <= 0)
            player.pos.x = 0;
          if (player.pos.x + player.physics_size.x >= screen_wh.x)
            player.pos.x = screen_wh.x - player.physics_size.x;
          if (player.pos.y <= 0)
            player.pos.y = 0;
          if (player.pos.y + player.physics_size.y >= screen_wh.y)
            player.pos.y = screen_wh.y - player.physics_size.y;

          // player attacks
          bool player_able_to_attacking = gs.editor_left_click_mode == EditorMode::PLAYER_ATTACK;
          if (player_able_to_attacking) {
            player::player_attack(app, gs, player, player_inventory, keys, delta_time_s, rnd);
          }

          bool player_alive = player.invulnerable || player.damage_taken < player.damage_able_to_be_taken;
          if (!player_alive)
            gs.game_running = GameRunning::GAME_OVER;
        }

        // update: vfx flash

        std::vector<std::reference_wrapper<GameObject2D>> vfx_flash_objects;
        vfx_flash_objects.insert(vfx_flash_objects.end(), gs.entities_player.begin(), gs.entities_player.end());
        vfx_flash_objects.insert(vfx_flash_objects.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
        vfx_flash_objects.push_back(gs.weapon_pistol);

        for (auto& obj : vfx_flash_objects) {
          if (obj.get().flash_time_left > 0.0f) {
            obj.get().flash_time_left -= delta_time_s;
            obj.get().colour = obj.get().flash_colour;
          } else {
            obj.get().colour = obj.get().original_colour;
          }
        }

        // update: vfx fade

        for (auto& go : gs.entities_vfx) {
          go.colour.a = go.time_alive_left / go.time_alive;
        }

        { // object lifecycle

          gameobject::update_entities_lifecycle(gs.entities_enemies, delta_time_s);
          gameobject::update_entities_lifecycle(gs.entities_bullets, delta_time_s);
          gameobject::update_entities_lifecycle(gs.entities_trees, delta_time_s);
          gameobject::update_entities_lifecycle(gs.entities_vfx, delta_time_s);

          // update light objects
          for (auto& light : gs.point_lights) {
            GameObject2D& obj = light.light_object;
            if (obj.do_lifecycle_timed) {
              obj.time_alive_left -= delta_time_s;
              if (obj.time_alive_left <= 0.0f) {
                obj.flag_for_delete = true;
              }
            }
          }

          // remove "attack" object before deleting "bullet" object (or any object that is cleaned up)
          // e.g when deleting "player" (in the future)
          std::vector<Attack>::iterator it = gs.attacks.begin();
          while (it != gs.attacks.end()) {
            const Attack& attack = (*it);
            int id = attack.entity_weapon_id;

            if (attack.weapon_type == ShopItem::PISTOL || attack.weapon_type == ShopItem::SHOTGUN ||
                attack.weapon_type == ShopItem::MACHINEGUN) {
              const auto& bullet = std::find_if(gs.entities_bullets.begin(),
                                                gs.entities_bullets.end(),
                                                [&id](const auto& obj) { return obj.id == id; });

              if (bullet != gs.entities_bullets.end() && bullet->flag_for_delete) {
                // remove the attack object
                it = gs.attacks.erase(it);
                continue;
              }
            }
            ++it;
          }

          // enemy has died
          for (auto& enemy : gs.entities_enemies) {
            if (enemy.flag_for_delete) {
              vfx::spawn_death_splat(rnd, enemy, enemy.sprite, gs.entities_vfx);
              gs.enemies_destroyed_this_wave += 1;
              gs.enemies_killed += 1;
              gs.p0_currency += 1;
            }
          }

          gameobject::erase_entities_that_are_flagged_for_delete(gs.entities_enemies, delta_time_s);
          gameobject::erase_entities_that_are_flagged_for_delete(gs.entities_bullets, delta_time_s);
          gameobject::erase_entities_that_are_flagged_for_delete(gs.entities_trees, delta_time_s);
          gameobject::erase_entities_that_are_flagged_for_delete(gs.entities_vfx, delta_time_s);

          std::vector<PointLight>::iterator it_1 = gs.point_lights.begin();
          while (it_1 != gs.point_lights.end()) {
            PointLight& light = (*it_1);
            if (light.light_object.flag_for_delete)
              it_1 = gs.point_lights.erase(it_1);
            else
              ++it_1;
          }
        }

        // Check if game phase is over
        {
          if (gs.entities_enemies.size() == 0 && gs.enemies_to_spawn_this_wave_left == 0) {
            gs.game_phase = GamePhase::SHOP;
          }
        }
      }

      // end game-active
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    {
      game::render(registry);

      //
      // lighting fbo
      //

      Framebuffer::bind_fbo(resources.fbo_lighting);
      // Debugging; // Framebuffer::default_fbo(); glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
      RenderCommand::set_depth_testing(false);
      RenderCommand::clear();
      {
        // angle, x, y for intersections
        std::vector<std::tuple<float, float, float>> intersections;

        std::vector<std::reference_wrapper<GameObject2D>> lit_entities;
        lit_entities.insert(lit_entities.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
        lit_entities.insert(lit_entities.end(), gs.entities_player.begin(), gs.entities_player.end());
        lit_entities.insert(lit_entities.end(), gs.entities_trees.begin(), gs.entities_trees.end());

        std::vector<std::reference_wrapper<PointLight>> lights = {};
        for (auto& light : gs.point_lights) {
          if (light.cast_shadows) {
            lights.push_back(light);
          }
        }
        for (auto& l : lights) {
          GameObject2D& light = l.get().light_object;
          glm::ivec2 light_pos = light.pos - gs.camera.pos;

          // this generates collision from the light point to the entities
          generate_intersections(gs.camera, light_pos, lit_entities, screen_wh, intersections);

          // now create a triangle fan from the generated information
          triangle_fan_renderer::reset_quad_vert_count();
          triangle_fan_renderer::begin_batch();

          resources.fan_shader.bind();
          resources.fan_shader.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
          resources.fan_shader.set_mat4("projection", projection);
          { // render light
            triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, light_pos);
            for (int i = 0; i < intersections.size(); i++) {
              auto& intersection = intersections[i];
              glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
              triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, pos);
            }
            auto& intersection = intersections[0];
            glm::vec2 pos = glm::vec2(std::get<1>(intersection), std::get<2>(intersection));
            triangle_fan_renderer::add_point_to_fan(gs.camera, resources.fan_shader, pos);
            triangle_fan_renderer::end_batch();
            triangle_fan_renderer::flush(resources.fan_shader);
          }
        }
      }
      //
      // main scene fbo
      //
      Framebuffer::bind_fbo(resources.fbo_main_scene);
      RenderCommand::set_clear_colour(background_colour);
      RenderCommand::set_depth_testing(false);
      RenderCommand::clear();
      {
        sprite_renderer::reset_quad_vert_count();
        sprite_renderer::begin_batch();
        resources.instanced_quad_shader.bind();
        resources.instanced_quad_shader.set_float("time", app.seconds_since_launch);
        resources.instanced_quad_shader.set_mat4("projection", projection);
        resources.instanced_quad_shader.set_bool("do_lighting", false);
        resources.instanced_quad_shader.set_bool("do_spritesheet", true);

        if (gs.game_running == GameRunning::ACTIVE || gs.game_running == GameRunning::PAUSED ||
            gs.game_running == GameRunning::GAME_OVER) {

          // This list defines the render order
          std::vector<std::reference_wrapper<GameObject2D>> renderables;
          renderables.insert(renderables.end(), gs.entities_vfx.begin(), gs.entities_vfx.end());
          renderables.insert(renderables.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
          renderables.insert(renderables.end(), gs.entities_bullets.begin(), gs.entities_bullets.end());
          renderables.insert(renderables.end(), gs.entities_player.begin(), gs.entities_player.end());
          renderables.push_back(gs.weapon_shovel);
          renderables.push_back(gs.weapon_pistol);
          renderables.push_back(gs.weapon_shotgun);
          renderables.push_back(gs.weapon_machinegun);
          renderables.insert(renderables.end(), gs.entities_trees.begin(), gs.entities_trees.end());

          for (auto& obj : renderables) {
            if (!obj.get().do_render)
              continue;
            sprite_renderer::draw_instanced_sprite(gs.camera, screen_wh, resources.instanced_quad_shader, obj.get());
          }

          sprite_renderer::draw_sprites_debug(
            gs.camera, screen_wh, renderables, resources.colour_shader, debug_line_colour);

        } // <!-- end GameRunning::Active -->
        sprite_renderer::end_batch();
        sprite_renderer::flush(resources.instanced_quad_shader);
      } // <!-- end main scene fbo -->

      //
      // default fbo, draw a texture to a quad
      //
      Framebuffer::default_fbo();
      RenderCommand::set_clear_colour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // hit damage colour
      RenderCommand::set_depth_testing(false);
      RenderCommand::clear();
      {
        sprite_renderer::reset_quad_vert_count();
        sprite_renderer::begin_batch();
        resources.instanced_quad_shader.bind();
        resources.instanced_quad_shader.set_float("time", app.seconds_since_launch);
        glm::mat4 flip = glm::mat4(1.0f); // flip because opengl textures have different axis
        flip = glm::rotate(flip, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        resources.instanced_quad_shader.set_mat4("projection", flip * projection);
        resources.instanced_quad_shader.set_bool("do_lighting", true);
        resources.instanced_quad_shader.set_int("screen_w", screen_wh.x);
        resources.instanced_quad_shader.set_int("screen_h", screen_wh.y);
        resources.instanced_quad_shader.set_bool("do_spritesheet", false);
        { // send light positions to shader
          const int max_lights = 32;
          int i = 0;
          for (i = 0; i < gs.point_lights.size(); i++) {
            if (i >= max_lights) {
              std::cout << "warning: too many lights" << std::endl;
              continue;
            }
            auto& light = gs.point_lights[i];
            glm::vec2 light_pos = light.light_object.pos;
            light_pos = glm::vec3(light_pos.x, glm::abs(light_pos.y - screen_wh.y), 0.0f); // flip

            resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(i) + "]", true);
            resources.instanced_quad_shader.set_vec3("light_pos[" + std::to_string(i) + "]",
                                                     glm::vec3(light_pos, 0.0f));
            resources.instanced_quad_shader.set_float("light_linear[" + std::to_string(i) + "]", light.linear);
            resources.instanced_quad_shader.set_float("light_quadratic[" + std::to_string(i) + "]", light.quadratic);
          }
          for (int j = max_lights - 1; j >= i; j--) {
            resources.instanced_quad_shader.set_bool("light_enabled[" + std::to_string(j) + "]", false);
          }
        }

        { // draw single quad as entire screen
          GameObject2D screen_object = gameobject::create_generic();
          screen_object.sprite = sprite::type::SQUARE;
          screen_object.render_size = glm::vec2(screen_wh.x, screen_wh.y);
          screen_object.tex_slot = tex_unit_main_scene;
          sprite_renderer::draw_instanced_sprite(gs.camera, screen_wh, resources.instanced_quad_shader, screen_object);
        }
        sprite_renderer::end_batch();
        sprite_renderer::flush(resources.instanced_quad_shader);
        // CHECK_OPENGL_ERROR(0);
      }
    }
    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    {
      // ui: shop window

      if (gs.game_phase == GamePhase::SHOP) {
        ImGui::Begin("Humble Wares", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

        shop::update_shop(gs.p0_currency,
                          resources.texture_ids[0], // kenny texture id
                          gs.shop,
                          gs.stats_pistol,
                          gs.stats_shotgun,
                          gs.stats_machinegun,
                          shop_refill_pistol_ammo,
                          shop_refill_shotgun_ammo,
                          shop_refill_machinegun_ammo,
                          gs.player_inventories,
                          gs.entities_player);

        if (ImGui::Button("Leave shop")) {
          enemy_spawner::next_wave(gs.enemies_to_spawn_this_wave, gs.enemies_to_spawn_this_wave_left, gs.wave);
          gs.game_phase = GamePhase::ATTACK;
        }
        ImGui::End();
      }

      // ui: options menu

      if (ui_esc_menu) {

        gs.game_running = GameRunning::PAUSED;
        ImGui::Begin("PAUSE MENU", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        {
          { // continue button
            if (ImGui::Button("Continue")) {
              gs.game_running = GameRunning::ACTIVE;
              ui_esc_menu = false;
            }
          }

          ImGui::Separator();
          ImGui::Text("Settings");
          bool temp;
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
              toggle_fullscreen(app, screen_wh, projection, resources.tex_id_lighting, resources.tex_id_main_scene);
              gs.point_lights.clear();
              add_lighting(gs, screen_wh);
            }
            ui_fullscreen = temp;
          }

          ImGui::Separator();
          { // restart button
            if (ImGui::Button("Restart")) {
              gs = reset_game(screen_wh);
            }
          }
          { // exit to desktop
            if (ImGui::Button("Exit to desktop")) {
              app.shutdown();
            }
          }
        }
        ImGui::End();
      }

      // ui: top menu bar

      if (ImGui::BeginMainMenuBar()) {

        ImGui::Text("WAVE %i", gs.wave);

        for (int i = 0; i < gs.entities_player.size(); i++) {
          GameObject2D& player = gs.entities_player[i];
          auto& p_inventory = gs.player_inventories[i];

          const glm::vec2 heart_icon_size = { 12.0f, 12.0f };

          {
            int empty_hearts_to_show = player.damage_taken;
            int full_hearts_to_show = player.damage_able_to_be_taken - player.damage_taken;
            std::array<ImVec2, 2> heart_full_uv = convert_sprite_to_uv(sprite_heart_4);
            std::array<ImVec2, 2> heart_empty_uv = convert_sprite_to_uv(sprite_heart_2);
            ImGui::SetCursorPosY(4.0f);
            for (int i = 0; i < full_hearts_to_show; i++) {
              ImGui::Image((ImTextureID)resources.texture_ids[0],
                           { heart_icon_size.x, heart_icon_size.y },
                           heart_full_uv[0],
                           heart_full_uv[1]);
            }
            for (int i = 0; i < empty_hearts_to_show; i++) {
              ImGui::Image((ImTextureID)resources.texture_ids[0],
                           { heart_icon_size.x, heart_icon_size.y },
                           heart_empty_uv[0],
                           heart_empty_uv[1]);
            }
          }

          ImGui::SetCursorPosY(0.0f);
          const glm::vec2 icon_size = { 20.0f, 20.0f };

          ImGui::Text("BOOST %.2fs", player.shift_boost_time_left);
          {
            std::array<ImVec2, 2> pistol_uv = convert_sprite_to_uv(sprite_pistol);
            ImGui::Image(
              (ImTextureID)resources.texture_ids[0], { icon_size.x, icon_size.y }, pistol_uv[0], pistol_uv[1]);
            std::string ammo_pistol_label = std::to_string(gs.stats_pistol.current_ammo);
            if (gs.stats_pistol.infinite_ammo) {
              ImGui::Text("INF");
            } else {
              ImGui::Text(ammo_pistol_label.c_str());
            }
          }
          {
            std::array<ImVec2, 2> shotgun_uv = convert_sprite_to_uv(sprite_shotgun);
            ImGui::Image(
              (ImTextureID)resources.texture_ids[0], { icon_size.x, icon_size.y }, shotgun_uv[0], shotgun_uv[1]);
            std::string ammo_shotgun_label = std::to_string(gs.stats_shotgun.current_ammo);
            ImGui::Text(ammo_shotgun_label.c_str());
          }
          {
            std::array<ImVec2, 2> machinegun_uv = convert_sprite_to_uv(sprite_machinegun);
            ImGui::Image(
              (ImTextureID)resources.texture_ids[0], { icon_size.x, icon_size.y }, machinegun_uv[0], machinegun_uv[1]);
            std::string ammo_machinegun_label = std::to_string(gs.stats_machinegun.current_ammo);
            ImGui::Text(ammo_machinegun_label.c_str());
          }

          ShopItem w = p_inventory[player.equipped_item_index];
          std::string wep = std::string("Weapon: ") + std::string(magic_enum::enum_name(w));
          ImGui::Text(wep.c_str());

          ImGui::SameLine(screen_wh.x - 170.0f);
          {
            float framerate = ImGui::GetIO().Framerate;
            float framerate_ms = 1000.0f / ImGui::GetIO().Framerate;
            ImGui::Text("FPS %f", framerate);
          }

          // ui: pause button
        }

        ImGui::EndMainMenuBar();
      }

      // ui: profiler

      if (debug_show_profiler) {
        ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
        ImGui::Separator();
        ImGui::Text("draw_calls: %i", sprite_renderer::get_draw_calls());
        profiler_panel::draw_timers(profiler, delta_time_s);
        ImGui::End();
      }

      // ui: imgui demo window

      if (debug_show_imgui_demo_window)
        ImGui::ShowDemoWindow(&debug_show_imgui_demo_window);

      // ui: debug game

      ImGui::Begin("Debug");
      {
        ImGui::Text("Player Pos: %i %i", gs.entities_player[0].pos.x, gs.entities_player[0].pos.y);
        ImGui::Text("Player Size: %i %i", gs.entities_player[0].render_size.x, gs.entities_player[0].render_size.y);
        ImGui::Text("Players: %i", gs.entities_player.size());
        ImGui::Text("Bullets: %i", gs.entities_bullets.size());
        ImGui::Text("Enemies: %i", gs.entities_enemies.size());
        ImGui::Text("Trees: %i", gs.entities_trees.size());
        ImGui::Text("Vfx: %i", gs.entities_vfx.size());
        ImGui::Text("Attacks: %i", gs.attacks.size());
        ImGui::Text("controllers %i", SDL_NumJoysticks());
        ImGui::BeginChild("GameRender");
        ImGui::Image((ImTextureID)resources.texture_ids[0],
                     { spritesheet_width, spritesheet_height },
                     ImVec2(0.0f, 0.0f),
                     ImVec2(1.0f, 1.0f));
        ImGui::EndChild();
      }
      ImGui::End();
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
