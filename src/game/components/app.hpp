#pragma once

#include "game/components/components.hpp"
#include "game/modules/ui_event_console/components.hpp"
#include "modules/events/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/networking/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/ui_editor_bar/components.hpp"
#include "modules/ui_networking/components.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_sprite_searcher/components.hpp"
#include "resources/audio.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

#include <entt/entt.hpp>

namespace game2d {

template<class T>
struct SetOnce
{
private:
  T state = false;
  bool was_set = false;

public:
  SetOnce(const T& start)
    : state(start){};

  void Set(const T& new_state)
  {
    if (was_set)
      return;
    was_set = false;
    state = new_state;
  };

  T Get() const { return state; }
};

// persistent between games
struct GameEditor
{
  Profiler profiler;
  SINGLETON_AudioComponent audio;
  SINGLETON_Animations animations;
  SINGLETON_ColoursComponent colours;
  SINGLETON_EditorComponent editor;
  SINGLETON_SpriteSearcher sprites;
  SINGLETON_Textures textures;
  SINGLETON_RendererInfo renderer;
  // networking
  SINGLETON_NetworkingUIComponent networking_ui;
  SINGLETON_ServerComponent server;
  SINGLETON_ClientComponent client;
};

// reset on gameover
struct Game
{
  entt::registry state;
  SINGLETON_EntityBinComponent dead;
  SINGLETON_EventConsoleLogComponent ui_events;
  SINGLETON_FixedUpdateInputHistory fixed_update_input;
  SINGLETON_InputComponent input;
  SINGLETON_PhysicsComponent physics;

  // beginning of the end
  SetOnce<bool> on_start{ true };
  SetOnce<bool> gameover{ false };
};

} // namespace game2d