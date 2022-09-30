// your header
#include "game.hpp"

// systems&components&helpers
#include "helpers/dungeon.hpp"
#include "modules/audio/system.hpp"
#include "modules/cursor/system.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/system.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/networking/system.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/system.hpp"
#include "modules/ui_editor_bar/components.hpp"
#include "modules/ui_editor_bar/system.hpp"
#include "modules/ui_editor_scene/system.hpp"
#include "modules/ui_editor_tilemap/components.hpp"
#include "modules/ui_editor_tilemap/system.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_networking/components.hpp"
#include "modules/ui_networking/system.hpp"
#include "modules/ui_physics/system.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_profiler/helpers.hpp"
#include "modules/ui_profiler/system.hpp"
#include "modules/ui_sprite_searcher/components.hpp"
#include "modules/ui_sprite_searcher/system.hpp"

#include "engine/app/application.hpp"
#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "game/simulate.hpp"
#include "game/systems/player_inputs.hpp"
#include "game/systems/resolve_collisions.hpp"
#include "game/systems/ui_player_inventory.hpp"
#include "modules/camera/components.hpp"
#include "resources/audio.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

// other lib
#include <glm/glm.hpp>

namespace game2d {

void
init_game_state(entt::registry& r)
{
  r.each([&r](auto entity) { r.destroy(entity); });
  ctx_reset<SINGLETON_PhysicsComponent>(r);
  ctx_reset<SINGLETON_GameOverComponent>(r);
  ctx_reset<SINGLETON_EntityBinComponent>(r);
  ctx_reset<SINGLETON_FixedUpdateInputHistory>(r);

  // reset editor tools?
  auto tilemap_ent = r.create();
  r.emplace<TilemapComponent>(tilemap_ent);

  create_hierarchy_root_node(r);
  create_gameplay(r, EntityType::free_cursor);

  // EntityType et = EntityType::shopkeeper;
  // auto shopkeeper = create_gameplay(r, et);
  // create_renderable(r, shopkeeper, et);

  // // stock up!
  // const auto& view = r.view<ShopKeeperComponent>();
  // view.each([&r](auto shop_entity, auto& shopkeeper) {
  //   create_item(r, EntityType::potion, shop_entity);
  //   create_item(r, EntityType::potion, shop_entity);
  //   create_item(r, EntityType::potion, shop_entity);
  //   create_item(r, EntityType::sword, shop_entity);
  //   create_item(r, EntityType::fire_sword, shop_entity);
  //   create_item(r, EntityType::shield, shop_entity);
  //   create_item(r, EntityType::stone, shop_entity);
  //   create_item(r, EntityType::crossbow, shop_entity);
  //   create_item(r, EntityType::bolt, shop_entity);
  //   create_item(r, EntityType::scroll_confusion, shop_entity);
  //   create_item(r, EntityType::scroll_fireball, shop_entity);
  //   create_item(r, EntityType::scroll_magic_missile, shop_entity);
  // });

  const int GRID_SIZE = 16;

  // players
  for (int i = 0; i < 1; i++) {
    EntityType et = EntityType::player;
    entt::entity e = create_gameplay(r, et);
    create_renderable(r, e, et);
  }

  std::cout << "creating dungeon...!" << std::endl;
  Dungeon d;
  generate_dungeon(r, d);
  entt::entity e = r.create();
  r.emplace<Dungeon>(e, d);
};

} // namespace game2d

void
game2d::init(entt::registry& r)
{
  const auto& app = r.ctx().at<engine::SINGLETON_Application>();

  // editor tools
  ctx_reset<Profiler>(r);
  ctx_reset<SINGLETON_EditorComponent>(r);
  ctx_reset<SINGLETON_SpriteSearcher>(r);
  // other
  ctx_reset<SINGLETON_Textures>(r);
  ctx_reset<SINGLETON_ColoursComponent>(r);
  init_sprite_system(r);
  init_render_system(r);
  init_input_system(r);
  init_audio_system(r);
  init_networking_system(r);

  // GOAL: remove init_game_state with the
  // map load/save functionality
  // eventually this game state should be savable to a file?
  // maybe just the visuals are in this file
  init_game_state(r);
};

void
game2d::fixed_update(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& p = r.ctx().at<Profiler>();
  {
    auto _ = time_scope(&p, "(all)", true);
    auto& input = r.ctx().at<InputComponent>();
    auto& fixed_input = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();

    // while offline, just clear out anything older than a tick
    // (until a gameplay system needs older input)
    fixed_input.history.clear();

    // move inputs from Update() to this FixedUpdate() tick
    fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_update_inputs);
    auto& inputs = fixed_input.history[fixed_input.fixed_tick];

    simulate(r, inputs, milliseconds_dt);

    fixed_input.fixed_tick += 1;

    // update_networking_system(r, milliseconds_dt);
  }
}

void
game2d::update(entt::registry& r, float dt)
{
  auto& p = r.ctx().at<Profiler>();
  const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
  auto& app = r.ctx().at<engine::SINGLETON_Application>();
  {
    auto _ = time_scope(&p, "game_tick");
    update_input_system(r);
    auto& input = r.ctx().at<InputComponent>();
    if (ri.viewport_process_events) {
      if (get_key_down(input, SDL_SCANCODE_R))
        init_game_state(r);
      if (get_key_down(input, SDL_SCANCODE_F))
        app.window.toggle_fullscreen();
      if (get_key_down(input, SDL_SCANCODE_ESCAPE))
        app.running = false;
    }

    // ... systems that always update
    {
      // update_cursor_system(r);
      update_audio_system(r);
    }

    // ... systems that update only if viewport is focused or hovered
    {
      if (ri.viewport_process_events) {
        // update_camera_system(r);
        update_player_inputs_system(r);
        update_cursor_system(r);
      }
    }
  };
  {
    // put rendering on thread?
    auto _ = time_scope(&p, "rendering");
    update_sprite_system(r, dt);
    update_render_system(r);
  };

  {
    auto _ = time_scope(&p, "ui"); // value always be a frame behind
    {
      static bool show_editor_ui = true;
      if (show_editor_ui) {
        update_ui_editor_bar_system(r);
        update_ui_editor_tilemap_system(r);
        update_ui_editor_scene_system(r);
        update_ui_physics_system(r);
        update_ui_hierarchy_system(r);
        update_ui_profiler_system(r);
        update_ui_sprite_searcher_system(r);
      }
      update_ui_player_inventory_system(r);
    }
    // update_ui_networking_system(r);
    // update_ui_main_menu_system(r);
  };

  // end frame
  {
    end_frame_render_system(r);
  }
};
