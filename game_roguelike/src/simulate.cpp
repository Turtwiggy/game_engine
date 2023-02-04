#include "simulate.hpp"

// game
#include "modules/ai/system.hpp"
#include "modules/combat/flash_sprite.hpp"
#include "modules/combat/take_damage.hpp"
#include "modules/dungeon/system.hpp"
#include "modules/items/intent_drop_item.hpp"
#include "modules/items/intent_equip_item.hpp"
#include "modules/items/intent_purchase_item.hpp"
#include "modules/items/intent_select_units_for_item.hpp"
#include "modules/items/intent_use_item.hpp"
#include "modules/player/player_controller.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/rpg_xp/stats.hpp"
#include "modules/rpg_xp/system.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_profiler/helpers.hpp"

#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/system.hpp"
#include "lifecycle/components.hpp"
#include "lifecycle/system.hpp"
#include "physics/components.hpp"
#include "physics/process_actor_actor.hpp"
#include "physics/process_move_objects.hpp"

void
game2d::simulate(GameEditor& editor, Game& game, const std::vector<InputEvent>& inputs, uint64_t milliseconds_dt)
{
  auto& p = editor.profiler;

  if (game.running_state == GameState::PAUSED)
    return; // skip all game logic
  if (game.running_state == GameState::GAMEOVER_LOSE)
    return; // skip all game logic
  if (game.running_state == GameState::GAMEOVER_WIN)
    return; // skip all game logic

  // process inputs in FixedUpdateInputHistory
  update_player_controller_system(editor, game, inputs, milliseconds_dt);

  // destroy objects
  update_lifecycle_system(game.dead, game.state, milliseconds_dt);

  {
    auto _ = time_scope(&p, "(physics)", true);
    auto& r = game.state;
    update_move_objects_system(r, milliseconds_dt); // move objects, checking collisions along way
    update_actor_actor_system(game.state, game.physics);
    update_resolve_collisions_system(game); // resolve collisions immediately
  }

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
    update_stats_system(editor, game);
    // flash_sprite after take_damage
    update_flash_sprite_system(editor, game, milliseconds_dt);
  }
  {
    auto _ = time_scope(&p, "(game_logic)-pathfinding/ai)", true);
    update_ai_system(editor, game, milliseconds_dt);
  }
  {
    auto _ = time_scope(&p, "(game_logic)-dungeon)", true);
    update_dungeon_system(editor, game);
  }
};