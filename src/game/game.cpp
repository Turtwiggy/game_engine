// your header
#include "game.hpp"

#include "game/components/app.hpp"

// systems&components&helpers
#include "engine/app/application.hpp"
#include "game/components/app.hpp"
#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "game/simulate.hpp"
#include "game/systems/player_inputs.hpp"
#include "game/systems/resolve_collisions.hpp"
#include "game/systems/ui_hp_bar.hpp"
#include "game/systems/ui_player_inventory.hpp"
#include "helpers/dungeon.hpp"
#include "modules/audio/system.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/system.hpp"
#include "modules/events/system.hpp"
#include "modules/networking/system.hpp"
#include "modules/renderer/system.hpp"
#include "modules/sprites/system.hpp"
#include "modules/ui_editor_tilemap/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_profiler/helpers.hpp"
#include "modules/ux_hover/components.hpp"

// other lib
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

namespace game2d {

void
init_game_state(const GameEditor& editor, Game& game)
{
  const auto& colours = editor.colours;

  game.state.clear();
  auto& r = game.state;

  // reset editor tools?
  auto tilemap_ent = r.create();
  r.emplace<TilemapComponent>(tilemap_ent);

  create_hierarchy_root_node(r);
  create_gameplay(r, EntityType::free_cursor);
  init_camera_system(editor, game);

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
    init_ui_hp_bar(r, e);
  }

  int size_x = 100;
  int size_y = 100;
  auto create_ui_arrow = [&](int sx, int sy, int x, int y) {
    engine::SRGBColour colour_default = colours.backdrop_red;
    engine::SRGBColour colour_hover = colours.red;
    EntityType et = EntityType::ui_action_card;
    entt::entity e = create_gameplay(r, et);
    SpriteComponent s = create_sprite(r, e, et);
    TransformComponent t = create_transform(r, e);
    SpriteColourComponent scc = create_colour(r, e, et);
    HoverComponent hc;
    t.scale.x = size_x;
    t.scale.y = size_y;
    t.position.x = x;
    t.position.y = y;
    s.x = sx;
    s.y = sy;
    scc.colour = engine::SRGBToLinear(colour_default);
    hc.hover_colour = colour_hover;
    hc.regular_colour = colour_default;
    r.emplace<SpriteComponent>(e, s);
    r.emplace<TransformComponent>(e, t);
    r.emplace<SpriteColourComponent>(e, scc);
    r.emplace<HoverComponent>(e, hc);
  };
  create_ui_arrow(23, 20, -96, 20);
  create_ui_arrow(24, 20, 250, 250);
  create_ui_arrow(25, 20, 500, 500);
  create_ui_arrow(26, 20, 100, 300);

  std::cout << "creating dungeon...!" << std::endl;
  Dungeon d;
  generate_dungeon(editor, game, d);
  entt::entity e = r.create();
  r.emplace<Dungeon>(e, d);
};

void
init(engine::SINGLETON_Application& app, GameEditor& editor, Game& game)
{
  game = {};
  init_sprite_system(editor);
  init_networking_system(editor);
  init_audio_system(editor);
  init_render_system(app, editor, game);
  init_input_system(game);

  // GOAL: remove init_game_state with the
  // map load/save functionality
  // eventually this game state should be savable to a file?
  // maybe just the visuals are in this file
  init_game_state(editor, game);
};

void
fixed_update(GameEditor& editor, Game& game, uint64_t milliseconds_dt)
{
  auto& p = editor.profiler;
  {
    auto _ = time_scope(&p, "(all)", true);
    auto& input = game.input;
    auto& fixed_input = game.fixed_update_input;

    // while offline, just clear out anything older than a tick
    // (until a gameplay system needs older input)
    fixed_input.history.clear();

    // move inputs from Update() to this FixedUpdate() tick
    fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_update_inputs);
    std::vector<InputEvent>& inputs = fixed_input.history[fixed_input.fixed_tick];

    simulate(editor, game, inputs, milliseconds_dt);

    fixed_input.fixed_tick += 1;

    // update_networking_system(r, milliseconds_dt);

    // reset game
    for (const InputEvent& i : inputs) {
      if (i.type == InputType::keyboard && i.key == static_cast<uint32_t>(SDL_SCANCODE_R) && !i.release) {
        init_game_state(editor, game);
      }
    }
  }
}

void
update(engine::SINGLETON_Application& app, GameEditor& editor, Game& game, float dt)
{
  auto& p = editor.profiler;
  const auto& ri = editor.renderer;
  {
    auto _ = time_scope(&p, "game_tick");
    update_input_system(r);

    auto& input = game.input;

    if (ri.viewport_process_events) {
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
        update_camera_system(r, dt);
        update_player_inputs_system(r);
        update_cursor_system(r);
        update_ux_hover_system(r);
        update_ui_hp_bar(r);
      }
    }
  };
  {
    // put rendering on thread?
    auto _ = time_scope(&p, "rendering");
    update_sprite_system(editor, game, dt);
    update_render_system(editor, game);
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
      // update_ui_player_inventory_system(r);
    }
    // update_ui_networking_system(r);
    // update_ui_main_menu_system(r);
  };

  // end frame
  {
    end_frame_render_system(r);
  }
};

} // namespace game2d
