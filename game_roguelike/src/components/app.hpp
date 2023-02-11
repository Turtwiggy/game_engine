#pragma once

#include "audio/components.hpp"
#include "events/components.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/ui_event_console/components.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_sprite_searcher/components.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

// Note: these arn't true singletons, as in the sense that they are
// single instances of components that could exist in the entt::registry
// where only the EntitySystem that is allowed to mutate the state.
// Everything else is allowed to read the data, but not mutate it.

// persistent between games
struct GameEditor
{
  Profiler profiler;
  SINGLETON_Animations animations;
  SINGLETON_ColoursComponent colours;
  SINGLETON_RendererInfo renderer;
  SINGLETON_SpriteSearcher sprites;
  SINGLETON_Textures textures;

  SINGLETON_AudioComponent audio{ {
    { "MENU", "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" },
    { "HIT", "assets/audio/usfx_1_4/VOICES/Martial_Arts_Male/VOICE_Martial_Art_Shout_03_mono.wav" },
    { "ATTACKED", "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" },
  } };
  entt::registry audio_state;

  // networking
  // SINGLETON_NetworkingUIComponent networking_ui;
  // SINGLETON_ServerComponent server;
  // SINGLETON_ClientComponent client;
};

enum class GameState
{
  START,
  RUNNING,
  PAUSED,
  GAMEOVER_LOSE,
  GAMEOVER_WIN,
};

// reset on gameover
struct Game
{
  entt::registry state;
  engine::RandomState rnd;
  SINGLETON_EntityBinComponent dead;
  SINGLETON_EventConsoleLogComponent ui_events;
  SINGLETON_FixedUpdateInputHistory fixed_update_input;
  SINGLETON_InputComponent input;
  SINGLETON_PhysicsComponent physics;

  GameState running_state = GameState::START;
  int live_dungeon_seed = 2;
  int live_dungeon_floor = 1;
};

} // namespace game2d