#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "modules/ui/ui_worldspace_text/helpers.hpp"

namespace game2d {

std::vector<std::string>
split_string_nearest_space(const std::string& str, const int& len)
{
  std::vector<std::string> results;

  size_t start = 0;
  while (start < str.size()) {
    size_t end = start + len;

    // Get the nearest ' ' char to the end of the sentence
    if (end < str.size() && str[end] != ' ') {
      const size_t space_pos = str.rfind(' ', end);
      if (space_pos != std::string::npos && space_pos > start)
        end = (int)space_pos;
    }

    // add the sentence so far
    results.push_back(str.substr(start, end - start));

    // onwards..!
    start = end;
    while (start < str.size() && str[start] == ' ')
      ++start; // Skip spaces
  }

  return results;
};

entt::entity
create_popup(entt::registry& r, glm::vec2 pos, std::string text)
{
  WorldspaceTextComponent wst_c;

  wst_c.layout = [text](entt::registry& r, entt::entity e, const WorldspaceTextComponent& data) {
    const auto text_col = hex_to_srgb("#ffffff");

    auto* font = get_inter_font(r);
    ImGui::PushFont(font, (float)FontSizes::SIZE_12);

    const auto im_crit_col = ImVec4{
      text_col.r / 255.0f,
      text_col.g / 255.0f,
      text_col.b / 255.0f,
      text_col.a / 255.0f,
    };

    const std::string label = std::format("{}", text);

    ImGui::TextColored(im_crit_col, "%s", label.c_str());

    ImGui::PopFont();
  };

  auto popup_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(popup_e);
  r.emplace<EntityTimedLifecycle>(popup_e, 1 * 3000);
  r.emplace<WiggleUpAndDown>(popup_e, WiggleUpAndDown{ .base_position = pos });
  set_position(r, popup_e, pos);
  return popup_e;
}

} // namespace game2d