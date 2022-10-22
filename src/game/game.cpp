// your header
#include "game.hpp"

#include "game/components/app.hpp"

// systems&components&helpers
#include "engine/app/application.hpp"
#include "engine/maths/grid.hpp"
#include "game/components/actors.hpp"
#include "game/modules/dungeon/system.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/resolve_collisions/system.hpp"
#include "game/modules/ui_event_console/system.hpp"
#include "game/modules/ui_main_menu/system.hpp"
#include "game/modules/ui_player/system.hpp"
#include "game/modules/ui_player_inventory/system.hpp"
#include "game/simulate.hpp"
#include "modules/audio/system.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/camera/system.hpp"
#include "modules/cursor/system.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/system.hpp"
#include "modules/networking/system.hpp"
#include "modules/renderer/system.hpp"
#include "modules/sprites/system.hpp"
#include "modules/ui_editor_bar/system.hpp"
#include "modules/ui_editor_scene/system.hpp"
#include "modules/ui_editor_tilemap/components.hpp"
#include "modules/ui_editor_tilemap/system.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_profiler/helpers.hpp"
#include "modules/ui_profiler/system.hpp"
#include "modules/ui_sprite_searcher/system.hpp"
#include "modules/ux_hover/components.hpp"
#include "modules/ux_hover/system.hpp"

// other lib
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

namespace game2d {

Game
init_game_state(GameEditor& editor)
{
  const auto& colours = editor.colours;

  Game game;
  auto& r = game.state;
  init_input_system(game);
  game.ui_events.events.push_back("Welcome!");

  create_hierarchy_root_node(r);

  EntityType et = EntityType::actor_shopkeeper;
  auto shopkeeper = create_gameplay(editor, r, et);
  // create_renderable(editor, r, shopkeeper, et);

  // stock up!
  const auto& view = r.view<ShopKeeperComponent>();
  view.each([&editor, &r](auto shop_entity, auto& shopkeeper) {
    create_item(editor, r, EntityType::potion, shop_entity);
    create_item(editor, r, EntityType::potion, shop_entity);
    create_item(editor, r, EntityType::potion, shop_entity);
    create_item(editor, r, EntityType::sword, shop_entity);
    create_item(editor, r, EntityType::fire_sword, shop_entity);
    create_item(editor, r, EntityType::shield, shop_entity);
    create_item(editor, r, EntityType::stone, shop_entity);
    create_item(editor, r, EntityType::crossbow, shop_entity);
    create_item(editor, r, EntityType::bolt, shop_entity);
    create_item(editor, r, EntityType::scroll_damage_nearest, shop_entity);
    create_item(editor, r, EntityType::scroll_damage_selected_on_grid, shop_entity);
  });

  // players
  for (int i = 0; i < 1; i++) {
    EntityType et = EntityType::actor_player;
    entt::entity e = create_gameplay(editor, r, et);
    create_renderable(editor, r, e, et);

    // give the player a sword
    entt::entity sword = create_item(editor, r, EntityType::sword, e);

    // equip it!
    r.emplace<WantsToEquip>(e, std::vector<entt::entity>{ sword });
  }

  int seed = 0;
  Dungeon d;
  game.ui_events.events.push_back("You are on floor: 0");

  generate_dungeon(editor, r, d, seed);
  entt::entity e = r.create();
  r.emplace<EntityTypeComponent>(e, EntityType::empty);
  r.emplace<TagComponent>(e, "dungeon");
  r.emplace<Dungeon>(e, d);

  // camera
  auto c = create_gameplay(editor, r, EntityType::camera);
  r.emplace<TransformComponent>(c);

  return game;
};

void
init(engine::SINGLETON_Application& app, GameEditor& editor, Game& game)
{
  init_sprite_system(editor);
  init_networking_system(editor);
  init_audio_system(editor);
  init_render_system(app, editor);

  // GOAL: remove init_game_state with the
  // map load/save functionality
  // eventually this game state should be savable to a file?
  // maybe just the visuals are in this file??
  game = init_game_state(editor);
};

void
fixed_update(GameEditor& editor, Game& game, uint64_t milliseconds_dt)
{
  auto& p = editor.profiler;
  {
    auto _ = time_scope(&p, "(all)", true);
    auto& input = game.input;
    auto& fixed_input = game.fixed_update_input;

    if (game.paused) {
      // ignore inputs
      input.unprocessed_inputs.clear();
    } else {

      // while offline, just clear out anything older than a tick
      // (until a gameplay system needs older input)
      fixed_input.history.clear();

      // move inputs from Update() to this FixedUpdate() tick
      fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);
      std::vector<InputEvent>& inputs = fixed_input.history[fixed_input.fixed_tick];

      simulate(editor, game, inputs, milliseconds_dt);
      fixed_input.fixed_tick += 1;

      // reset game
      for (const InputEvent& i : inputs) {
        if (i.type == InputType::keyboard && i.key == static_cast<uint32_t>(SDL_SCANCODE_R) &&
            i.state == InputState::press) {
          game = init_game_state(editor);
        }
      }
    }

    // update_networking_system(r, milliseconds_dt);
  }
}

void
update(engine::SINGLETON_Application& app, GameEditor& editor, Game& game, float dt)
{
  auto& p = editor.profiler;
  const auto& ri = editor.renderer;
  {
    auto _ = time_scope(&p, "game_tick");
    update_input_system(app, editor, game);
    update_camera_system(editor, game, dt);
    update_audio_system(editor);
    update_cursor_system(editor, game);
    update_ux_hover_system(editor, game);
  };

  {
    // put rendering on thread?
    auto _ = time_scope(&p, "rendering");
    update_sprite_system(editor, game, dt);
    update_render_system(editor, game);
  };

  {
    auto _ = time_scope(&p, "ui"); // value for ui always be a frame behind
    update_ui_event_console(editor, game);
    update_ui_player_system(editor, game);
    update_ui_player_inventory_system(editor, game);
    update_ui_main_menu_system(app, editor, game);

    static bool show_editor_ui = true;
    if (show_editor_ui) {
      update_ui_editor_bar_system(editor, game);
      update_ui_hierarchy_system(editor, game);
      update_ui_profiler_system(editor, game);
      // update_ui_editor_scene_system(editor, game);
      update_ui_editor_tilemap_system(editor, game);
      update_ui_sprite_searcher_system(editor, game);
      // update_ui_networking_system(editor, game);
    }
  };

  // end frame
  {
    auto& r = game.state;
    end_frame_render_system(r);
  }
};

} // namespace game2d
