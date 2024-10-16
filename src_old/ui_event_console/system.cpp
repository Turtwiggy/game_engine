#include "system.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_event_console/components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_event_console_system(entt::registry& r)
{
  const auto evts_e = get_first<SINGLE_EventConsoleLogComponent>(r);
  if (evts_e == entt::null)
    return;
  const auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  static int events = 0;
  const int new_events = (int)evts.events.size();

  if (events == 0 && new_events == 0)
    return; // hide ui

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoBackground;

  const ImVec2 size{ 300, 200 };

  // top right
  const ImVec2 pos{ ri.viewport_size_render_at.x - size.x, 0 };
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0, 0 });
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);

  ImGui::Begin("Log", NULL, flags);

  ImGui::SeparatorText("Log");

  for (const auto& evt : evts.events)
    ImGui::Text("%s", evt.c_str());

  // Display bottom when new event occurs
  if (new_events != events) {
    ImGui::SetScrollHereY(1.0f);
    events = new_events;
  }

  ImGui::End();
}

} // namespace game2d