#include "system.hpp"

#include "components.hpp"
#include "events/helpers/mouse.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_event_console(GameEditor& editor, Game& game)
{
  // debug
  // if (get_mouse_mmb_press())
  //   game.ui_events.events.push_back("MMB clicked");

  static int events = 0;
  static int new_events = 0;
  new_events = game.ui_events.events.size();

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoMove;

  ImGuiWindowClass window_class;
  window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
  ImGui::SetNextWindowClass(&window_class);

  static bool show_events = true;
  ImGui::Begin("GameEvents", &show_events, flags);

  for (const auto& evt : game.ui_events.events)
    ImGui::Text("[event] %s", evt.c_str());

  // Display bottom when new event occurs
  if (new_events != events) {
    ImGui::SetScrollHereY(1.0f);
    events = new_events;
  }

  ImGui::End();
}

} // namespace game2d