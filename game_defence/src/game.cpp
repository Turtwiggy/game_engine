#include "game.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "lifecycle/system.hpp"
#include "maths/maths.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/camera/system.hpp"
#include "modules/combat/components.hpp"
#include "modules/enemy/system.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/helpers.hpp"
#include "modules/gameover/system.hpp"
#include "modules/health/components.hpp"
#include "modules/physics_box2d/components.hpp"
#include "modules/physics_box2d/system.hpp"
#include "modules/player/system.hpp"
#include "modules/spawner/components.hpp"
#include "modules/spawner/system.hpp"
#include "modules/turret/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_economy/components.hpp"
#include "modules/ui_economy/system.hpp"
#include "modules/ui_gameover/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_main_menu/system.hpp"
#include "modules/ui_prefabs/system.hpp"
#include "renderer/components.hpp"
#include "renderer/system.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"
#include "ui_profiler/components.hpp"
#include "ui_profiler/helpers.hpp"
#include "ui_profiler/system.hpp"

void
game2d::init(engine::SINGLETON_Application& app, b2World& world, entt::registry& r)
{
  {
    SINGLETON_Animations anims;
    SINGLETON_Textures textures;
    {
      Texture kenny_texture;
      kenny_texture.path = std::string("assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");
      kenny_texture.spritesheet_path = std::string("assets/config/spritemap_kennynl.json");
      load_sprites(anims.animations, kenny_texture.spritesheet_path);
      textures.textures.push_back(kenny_texture);
    }
    init_textures(textures);
    r.emplace<SINGLETON_Textures>(r.create(), textures);
    r.emplace<SINGLETON_Animations>(r.create(), anims);
  }
  {
    SINGLETON_AudioComponent audio;
    // audio.sounds.push_back({ "MENU", "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" });
    // audio.sounds.push_back({ "HIT", "assets/audio/usfx_1_4/VOICES/Martial_Arts_Male/VOICE_Martial_Art_Shout_03_mono.wav"
    // }); audio.sounds.push_back({ "ATTACKED", "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" });
    r.emplace<SINGLETON_AudioComponent>(r.create(), audio);
  }

  r.emplace<engine::RandomState>(r.create());
  r.emplace<Profiler>(r.create());
  r.emplace<SINGLETON_RendererInfo>(r.create());
  r.emplace<SINGLETON_FixedUpdateInputHistory>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());

  auto& textures = get_first_component<SINGLETON_Textures>(r).textures;
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  auto& input = get_first_component<SINGLETON_InputComponent>(r);
  init_audio_system(audio);
  init_render_system(app, ri, textures);
  init_input_system(input, r);

  restart_game(r, world);
}

void
game2d::fixed_update(entt::registry& game, b2World& world, const uint64_t milliseconds_dt)
{
  auto& p = get_first_component<Profiler>(game);
  auto _ = time_scope(&p, "fixed_update()", true);

  auto& input = get_first_component<SINGLETON_InputComponent>(game);
  auto& fixed_input = get_first_component<SINGLETON_FixedUpdateInputHistory>(game);
  fixed_input.history.clear();

  // move inputs from Update() to this FixedUpdate() tick
  fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);
  const auto& inputs = fixed_input.history[fixed_input.fixed_tick];

  // allow gameover/restart requests to be processed
  update_gameover_system(game, world);

  // dont tick game logic if paused
  auto& state = get_first_component<SINGLETON_GameStateComponent>(game);
  if (state.state == GameState::PAUSED)
    return; // note: this ignores inputs
  auto& gameover = get_first_component<SINGLETON_GameOver>(game);
  if (gameover.game_is_over)
    return;

  // destroy/create objects
  update_lifecycle_system(game, world, milliseconds_dt);

  // update physics
  {
    auto _ = time_scope(&p, "(physics)-tick", true);
    update_physics_box2d_system(game, world, milliseconds_dt);
  }
  // resolve collisions
  {
    auto _ = time_scope(&p, "(physics)-collisions", true);
    auto& dead = get_first_component<SINGLETON_EntityBinComponent>(game);
    auto& econ = get_first_component<SINGLETON_Economy>(game);

    for (b2Contact* contact = world.GetContactList(); contact; contact = contact->GetNext()) {
      // contact->IsTouching() // should check this if using anything other than aabb

      entt::entity a = (entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
      entt::entity b = (entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

      if (!game.valid(a) || !game.valid(b))
        continue;

      const auto& a_type = game.get<EntityTypeComponent>(a).type;
      const auto& b_type = game.get<EntityTypeComponent>(b).type;

      const auto& collision_of_interest = [](const entt::entity& a_ent,
                                             const entt::entity& b_ent,
                                             const EntityType& a,
                                             const EntityType& b,
                                             const EntityType& a_actual,
                                             const EntityType& b_actual) -> std::pair<entt::entity, entt::entity> {
        if (a == a_actual && b == b_actual)
          return { a_ent, b_ent };
        if (a == b_actual && b == a_actual)
          return { b_ent, a_ent };
        return { entt::null, entt::null };
      };

      // bullet-enemy collision
      {
        const auto& [actor_enemy, actor_bullet] =
          collision_of_interest(a, b, a_type, b_type, EntityType::actor_enemy, EntityType::actor_bullet);
        if (actor_enemy != entt::null && actor_bullet != entt::null) {
          dead.dead.emplace(actor_enemy);
          dead.dead.emplace(actor_bullet);
          econ.kills += 1;
        }
      }

      // bullet-player collision
      {
        const auto& [actor_bullet, actor_player] =
          collision_of_interest(a, b, a_type, b_type, EntityType::actor_bullet, EntityType::actor_player);
        if (actor_bullet != entt::null && actor_player != entt::null) {
          dead.dead.emplace(actor_bullet);
        }
      }

      // player-enemy collision
      {
        const auto& [actor_player, actor_enemy] =
          collision_of_interest(a, b, a_type, b_type, EntityType::actor_player, EntityType::actor_enemy);
        if (actor_player != entt::null && actor_enemy != entt::null) {
          {
            const auto& enemy_atk = game.get<AttackComponent>(actor_enemy);
            // kill enemy
            dead.dead.emplace(actor_enemy);
            // player takes damage
            auto& hp = game.get<HealthComponent>(actor_player);
            hp.hp -= enemy_atk.damage;
            hp.hp = glm::max(0, hp.hp);
          }
        }
      }

      // hearth-enemy collision
      {
        const auto& [actor_hearth, actor_enemy] =
          collision_of_interest(a, b, a_type, b_type, EntityType::actor_hearth, EntityType::actor_enemy);
        if (actor_hearth != entt::null && actor_enemy != entt::null) {
          const auto& enemy_atk = game.get<AttackComponent>(actor_enemy);
          auto& hearth_hp = game.get<HealthComponent>(actor_hearth);
          hearth_hp.hp -= enemy_atk.damage;
          hearth_hp.hp = glm::max(0, hearth_hp.hp);
          dead.dead.emplace(actor_enemy);
        }
      }
    }
  }

  // update gamelogic
  {
    auto _ = time_scope(&p, "fixed-update-game-logic", true);
    update_player_controller_system(game, milliseconds_dt);
    update_enemy_system(game, milliseconds_dt);
    update_turret_system(game, milliseconds_dt);
    update_spawner_system(game, milliseconds_dt);
  }

  fixed_input.fixed_tick += 1;
}

void
game2d::update(engine::SINGLETON_Application& app, entt::registry& r, const b2World& world, const float dt)
{
  auto& p = get_first_component<Profiler>(r);
  auto _ = time_scope(&p, "update()");

  {
    auto _ = time_scope(&p, "game_tick");
    auto& input = get_first_component<SINGLETON_InputComponent>(r);
    auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
    update_input_system(app, input);
    update_camera_system(r, dt);
    update_audio_system(audio);
  };

  {
    auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    auto& texs = get_first_component<SINGLETON_Textures>(r).textures;
    update_render_system(r, texs);
  }

  // UI
  update_ui_main_menu_system(app, r);
  update_ui_prefabs_system(r);
  update_ui_controller_system(r);
  update_ui_economy_system(r);
  update_ui_gameover_system(r);
  update_ui_hierarchy_system(r);

  static bool show_editor_ui = false;
  if (show_editor_ui) {
    update_ui_profiler_system(r, world);
  }

  end_frame_render_system(r);
};
