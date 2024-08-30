#include "helpers.hpp"

#include "audio/components.hpp"
#include "entt/helpers.hpp"

#include "imgui.h"

namespace game2d {

void
play_sound_if_hovered(entt::registry& r, std::vector<std::string>& hovered_buttons, const std::string& label)
{
  const bool hovered = ImGui::IsItemHovered();
  const auto it = std::find(hovered_buttons.begin(), hovered_buttons.end(), label);

  // does not contain hovered but it's hovered.
  if (it == hovered_buttons.end() && hovered) {

    // only one button can be hovered
    if (hovered_buttons.size() != 0)
      hovered_buttons.clear();

    hovered_buttons.push_back(label);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_HOVER_01" });
  }

  // does contain hovered but it's not hovered.
  if (it != hovered_buttons.end() && !hovered)
    hovered_buttons.erase(it);
};

} // namespace game2d