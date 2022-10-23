#include "simulate.hpp"

// game
#include "game/modules/ai/system.hpp"
#include "game/modules/combat/take_damage.hpp"
#include "game/modules/dungeon/system.hpp"
#include "game/modules/fov/system.hpp"
#include "game/modules/items/intent_drop_item.hpp"
#include "game/modules/items/intent_equip_item.hpp"
#include "game/modules/items/intent_purchase_item.hpp"
#include "game/modules/items/intent_select_units_for_item.hpp"
#include "game/modules/items/intent_use_item.hpp"
#include "game/modules/player/player_controller.hpp"
#include "game/modules/player/player_stats.hpp"
#include "game/modules/resolve_collisions/system.hpp"
#include "game/modules/rpg_xp/system.hpp"

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

  // process inputs in FixedUpdateInputHistory
  update_player_controller_system(editor, game, inputs, milliseconds_dt);

  auto _ = time_scope(&p, "(game_logic)-dungeon", true);
  update_dungeon_system(editor, game);

  // destroy objects
  update_lifecycle_system(editor, game, milliseconds_dt);

  {
    auto _ = time_scope(&p, "(physics-move-objs)", true);
    // move objects, checking collisions along way
    auto& r = game.state;
    update_move_objects_system(r, milliseconds_dt);
  }
  {
    auto _ = time_scope(&p, "(physics-actor-actor)", true);
    // generate all collisions between actor-actor objects
    update_actor_actor_system(editor, game);
  }

  // resolve collisions immediately
  update_resolve_collisions_system(game);

  // game logic
  {
    auto _ = time_scope(&p, "(game_logic)-misc", true);
    update_select_units_for_item_system(editor, game);
    update_intent_use_item_system(editor, game);
    update_intent_purchase_item_system(editor, game);
    update_intent_drop_item_system(editor, game);
    update_intent_equip_item_system(editor, game);
    update_take_damage_system(editor, game);
    update_rpg_system(editor, game);
    update_player_stats_system(editor, game);
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