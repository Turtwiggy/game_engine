#include "ui_survive_xp_bar_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/renderer/components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_survive_xp_bar_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  const auto size = ImVec2(ri.viewport_size_render_at.x, 40);
  const auto size_half_y = ImVec2{ size.x, size.y / 2.0f };

  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);

  // data
  const int current_xp = sxp_c.xp;
  const float max_xp = sxp_c.xp_for_next_level;
  const float progress = current_xp / max_xp;

  // Define colors for the XP bar
  const ImU32 bg = IM_COL32(50, 50, 50, 255);
  const ImU32 fg = IM_COL32(100, 200, 100, 255);
  const auto text_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  ImGui::Begin("XPbar", NULL, flags);

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 spos = ImGui::GetCursorScreenPos();
  draw_list->AddRectFilled(spos, ImVec2(spos.x + size.x, spos.y + size_half_y.y), bg, 4.0f);
  draw_list->AddRectFilled(spos, ImVec2(spos.x + size.x * progress, spos.y + size_half_y.y), fg, 4.0f);

  {
    ImGuiStyle& style = ImGui::GetStyle();
    // std::string label = std::format("{} / {}", int(current_xp), max_xp);
    std::string label = std::format("LEVEL {}", sxp_c.level);
    const float alignment = 0.5f;
    const auto text_size = ImGui::CalcTextSize(label.c_str());
    const float text_size_x = text_size.x + style.FramePadding.x * 2.0f;
    const float text_size_y = text_size.y + style.FramePadding.y * 2.0f;

    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - text_size_x) * alignment;
    if (off > 0.0f)
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    ImGui::SetCursorPosY(size_half_y.y + (0.5f * text_size_y)); // below bar

    ImGui::TextColored(text_col, "%s", label.c_str());
  }

  ImGui::End();
}

} // namespace game2d