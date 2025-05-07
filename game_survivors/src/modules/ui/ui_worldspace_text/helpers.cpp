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

void
create_popup(entt::registry& r, glm::vec2 pos, std::string text)
{
  WorldspaceTextComponent wst_c;

  wst_c.layout = [text](entt::registry& r, entt::entity e, const WorldspaceTextComponent& data) {
    const auto text_col = hex_to_srgb("#ffffff");

    const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
    const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SMALL : FontSize::TEXT_SMALL_SCALED;
    auto* font = get_inter_font(r, font_enum);
    ImGui::PushFont(font);

    const auto im_crit_col = ImVec4{
      text_col.r / 255.0f,
      text_col.g / 255.0f,
      text_col.b / 255.0f,
      text_col.a / 255.0f,
    };

    std::string label = std::format("{}", text);

    const auto ui_wh = ImGui::GetContentRegionAvail();
    const auto ui_tl = ImGui::GetCursorPos();
    const auto ui_txt_size = ImGui::CalcTextSize(label.c_str());
    ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5f) - (ui_txt_size.x * 0.5f));
    ImGui::SetCursorPosY(ui_tl.y + (ui_wh.y * 0.5f) - (ui_txt_size.y * 0.5f));

    ImGui::TextColored(im_crit_col, "%s", label.c_str());

    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
  wst_c.flags |= ImGuiWindowFlags_NoBackground;
  wst_c.flags |= ImGuiWindowFlags_NoSavedSettings;

  auto popup_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(popup_e);
  r.emplace<EntityTimedLifecycle>(popup_e, 1 * 3000);
  r.emplace<WiggleUpAndDown>(popup_e, WiggleUpAndDown{ .base_position = pos });
  set_position(r, popup_e, pos);
}

} // namespace game2d