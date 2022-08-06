#include "game_tick.hpp"

#include "game/systems/player_inputs.hpp"

// systems&components&helpers
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/system.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/lifecycle/system.hpp"
#include "modules/networking/system.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/process_actor_actor.hpp"
#include "modules/physics/process_move_objects.hpp"

// game systems
#include "game/create_entities.hpp"

void
game2d::simulate(entt::registry& r, const std::vector<InputEvent>& inputs, uint64_t milliseconds_dt)
{
  // process inputs in FixedUpdateInputHistory
  update_player_inputs_system(r, inputs);

  // destroy objects
  update_lifecycle_system(r, milliseconds_dt);

  // move objects, checking collisions along way
  update_move_objects_system(r, milliseconds_dt);

  // generate all collisions between actor-actor objects
  update_actor_actor_system(r);
};