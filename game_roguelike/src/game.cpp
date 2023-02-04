// your header
#include "game.hpp"

#include "components/app.hpp"

// systems&components&helpers
#include "app/application.hpp"
#include "camera/components.hpp"
#include "camera/helpers.hpp"
#include "components/actors.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/system.hpp"
#include "maths/grid.hpp"
#include "modules/ai/system.hpp"
#include "modules/audio/system.hpp"
#include "modules/camera/system.hpp"
#include "modules/cursor/system.hpp"
#include "modules/dungeon/helpers.hpp"
#include "modules/dungeon/system.hpp"
#include "modules/fov/system.hpp"
#include "modules/items/components.hpp"
#include "modules/player/components.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/sprites/system.hpp"
#include "modules/ui_editor_colour_palette/system.hpp"
#include "modules/ui_editor_scene/system.hpp"
#include "modules/ui_editor_tilemap/components.hpp"
#include "modules/ui_editor_tilemap/system.hpp"
#include "modules/ui_event_console/system.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_main_menu/system.hpp"
#include "modules/ui_player/system.hpp"
#include "modules/ui_profiler/helpers.hpp"
#include "modules/ui_profiler/system.hpp"
#include "modules/ui_screen_gameover/system.hpp"
#include "modules/ui_screen_welcome/system.hpp"
#include "modules/ui_shop/system.hpp"
#include "modules/ui_sprite_searcher/system.hpp"
#include "modules/ux_hover/components.hpp"
#include "modules/ux_hover/system.hpp"
#include "renderer/helpers.hpp"
#include "renderer/system.hpp"
#include "resources/textures.hpp"
#include "simulate.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

#include <algorithm>
#include <vector>

namespace game2d {

Game
init_game_state(GameEditor& editor)
{
  Game game;
  int default_seed = 1;
  int default_floor = 0;
  transfer_old_state_generate_dungeon(editor, game, default_seed, default_floor);

  // just because this is the demo i.e. menu screen:

  for (const auto [entity, type] : game.state.view<EntityTypeComponent>().each()) {
    if (type.type == EntityType::tile_type_exit)
      game.state.destroy(entity);
  }

  return game;
};

void
init(engine::SINGLETON_Application& app, GameEditor& editor, Game& game)
{
  // init_networking_system(editor);
  init_audio_system(editor);
  init_textures(editor.animations, editor.textures);
  {
    auto texs = convert_tag_textures_to_textures(editor.textures);
    init_render_system(app, editor.renderer, texs);
  }

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

    // if (game.running_state != GameState::RUNNING) {
    //   // ignore inputs
    //   input.unprocessed_inputs.clear();
    // } else
    {
      // while offline, just clear out anything older than a tick
      // (until a gameplay system needs older input)
      fixed_input.history.clear();

      // move inputs from Update() to this FixedUpdate() tick
      fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);
      const auto& inputs = fixed_input.history[fixed_input.fixed_tick];

      simulate(editor, game, inputs, milliseconds_dt);
      fixed_input.fixed_tick += 1;
    }

    // update_networking_system(r, milliseconds_dt);
  }
}

void
update(engine::SINGLETON_Application& app, GameEditor& editor, Game& game, float dt)
{
  auto& p = editor.profiler;
  {
    auto _ = time_scope(&p, "game_tick");
    // general
    update_input_system(app, game.input);
    update_camera_system(editor, game, dt);
    update_audio_system(editor, game);
    // game
    update_cursor_system(editor, game);
    update_ux_hover_system(editor, game);
    update_tile_fov_system(editor, game);
  };

  {
    {
      auto& ri = editor.renderer;
      if (get_key_held(game.input, SDL_SCANCODE_T)) {
        float seconds_since_launch = app.ms_since_launch / 1000.0f;
        ri.instanced.bind();
        ri.instanced.set_bool("shake", true);
        ri.instanced.set_float("time", seconds_since_launch);
      };
      if (get_key_up(game.input, SDL_SCANCODE_T)) {
        ri.instanced.bind();
        ri.instanced.set_bool("shake", false);
      };
    }

    // put rendering on thread?
    auto _ = time_scope(&p, "rendering");
    update_sprite_system(editor, game, dt);
    auto texs = convert_tag_textures_to_textures(editor.textures);
    update_render_system(editor.renderer, editor.colours.lin_background, texs, game.state);
  };

  {
    auto _ = time_scope(&p, "ui"); // value for ui always be a frame behind
    update_ui_event_console(editor, game);
    update_ui_player_system(editor, game);
    update_ui_welcome_system(editor, game);
    update_ui_main_menu_system(app, editor, game);
    update_ui_gameover_system(editor, game);

    static bool show_editor_ui = false;
    if (show_editor_ui) {
      update_ui_shop_system(editor, game);
      // update_ui_editor_bar_system(editor, game);
      // update_ui_hierarchy_system(editor, game);
      update_ui_profiler_system(editor, game);
      // update_ui_editor_scene_system(editor, game);
      update_ui_editor_tilemap_system(editor, game);
      update_ui_editor_colour_palette_system(editor, game);
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
