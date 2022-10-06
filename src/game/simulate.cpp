#include "simulate.hpp"

// game
#include "game/helpers/ai.hpp"
#include "game/helpers/dungeon.hpp"
#include "game/helpers/fov.hpp"
#include "game/systems/intent_use_item.hpp"
#include "game/systems/player_controller.hpp"
#include "game/systems/resolve_collisions.hpp"

// modules
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/system.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/lifecycle/system.hpp"
#include "modules/networking/system.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/process_actor_actor.hpp"
#include "modules/physics/process_move_objects.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_profiler/helpers.hpp"

void
game2d::simulate(GameEditor& editor, Game& game, const std::vector<InputEvent>& inputs, uint64_t milliseconds_dt)
{
  auto& p = editor.profiler;
  auto& r = game.state;

  // process inputs in FixedUpdateInputHistory
  update_player_controller_system(r, inputs);

  // destroy objects
  update_lifecycle_system(r, milliseconds_dt);

  {
    auto _ = time_scope(&p, "(physics-move-objs)", true);
    // move objects, checking collisions along way
    update_move_objects_system(r, milliseconds_dt);
  }
  {
    auto _ = time_scope(&p, "(physics-actor-actor)", true);
    // generate all collisions between actor-actor objects
    update_actor_actor_system(r);
  }

  // resolve collisions immediately
  update_resolve_collisions_system(r);

  // game logic
  {
    update_intent_use_item_system(r);
  }
  {
    auto _ = time_scope(&p, "(game_logic)-dungeon", true);
    update_dungeon_system(editor, game);
  }
  {
    auto _ = time_scope(&p, "(game_logic)-fov", true);
    update_tile_fov_system(editor, game);
  }
  {
    auto _ = time_scope(&p, "(game_logic)-pathfinding/ai)", true);
    update_ai_system(editor, game, milliseconds_dt);
  }
};