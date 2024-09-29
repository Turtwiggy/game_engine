#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "imgui.h"
#include "modules/combat/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include <fmt/core.h>

namespace game2d {

void
handle_damage_event_for_ui(entt::registry& r, const DamageEvent& evt)
{
  const auto from_e = evt.from; // previously bullet, now player?
  const auto to_e = evt.to;
  const auto amount = evt.amount;

  // add a new entry to the UI_BufferComponent...
  auto& buffer_c = r.get_or_emplace<UI_BufferComponent>(to_e);
  TimedEntry entry;
  entry.data = std::to_string(amount);
  entry.time_left_max = 2.0f;
  entry.time_left = entry.time_left_max;
  buffer_c.entries.push_back(entry);

  fmt::println("adding damage event for ui...");
};

void
update_ui_combat_damage_numbers_system(entt::registry& r, const float dt)
{
  const auto& view = r.view<HealthComponent>();
  for (const auto& [e, hp_c] : view.each()) {
    auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
    auto& ui = r.get_or_emplace<UI_BufferComponent>(e);

    // tick down the timer
    std::for_each(ui.entries.begin(), ui.entries.end(), [dt](TimedEntry& entry) { entry.time_left -= dt; });

    // remove the out of time entries...
    const auto to_remove = [](const TimedEntry& entry) { return entry.time_left <= 0.0f; };
    ui.entries.erase(std::remove_if(ui.entries.begin(), ui.entries.end(), to_remove), ui.entries.end());

    worldspace_ui.flags = ImGuiWindowFlags_NoDecoration;
    worldspace_ui.flags |= ImGuiWindowFlags_NoBackground;
    worldspace_ui.flags |= ImGuiWindowFlags_AlwaysAutoResize;
    worldspace_ui.flags |= ImGuiWindowFlags_NoDocking;
    worldspace_ui.flags |= ImGuiWindowFlags_NoFocusOnAppearing;

    const auto size = get_size(r, e);
    worldspace_ui.offset.x = 0.0f;
    worldspace_ui.offset.y = -1.35f * size.y; // place above

    worldspace_ui.layout = [&ui]() {
      const int max_hits_to_display = 3;

      // The last entry will always have the most recent data in it.
      // We want to display that at the bottom...
      // so just take the last 3 entries, in order, and print them top to bottom.

      const int start_idx = ui.entries.size() - max_hits_to_display;
      for (int i = start_idx; i < (int)ui.entries.size(); i++) {
        if (i < 0)
          continue;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        const auto& entry = ui.entries[i];
        const std::string label = entry.data;

        // centered
        const auto avail = ImGui::GetContentRegionAvail().x;
        const auto& style = ImGui::GetStyle();
        const auto alignment = 0.5f;
        const auto size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;
        const auto off = (avail - size) * alignment;
        if (off > 0.0f)
          ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

        // todo: make colour (of all text) flash if crit
        const float percent = entry.time_left / entry.time_left_max;
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, percent), "%s", label.c_str());

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
      }
    };

    //
  }
}

} // namespace game2d