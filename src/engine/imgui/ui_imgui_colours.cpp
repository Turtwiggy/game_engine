#include "pch.hpp"

#include "ui_imgui_colours.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_imgui_colours_system(entt::registry& r)
{
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  const auto& ui_state = gesert_menubar_state(menu_c, "ImGui Colour Editor");
  if (!ui_state.enabled)
    return;

  ImGui::Begin("ColourEditor");

  // CINDER IMGUI
  static float var_ImGuiCol_Text[4] = { 0.86f, 0.93f, 0.89f, 0.78f };
  static float var_ImGuiCol_TextDisabled[4] = { 0.86f, 0.93f, 0.89f, 0.28f };
  static float var_ImGuiCol_WindowBg[4] = { 0.13f, 0.14f, 0.17f, 1.00f };
  static float var_ImGuiCol_Border[4] = { 0.31f, 0.31f, 1.00f, 0.00f };
  static float var_ImGuiCol_BorderShadow[4] = { 0.00f, 0.00f, 0.00f, 0.00f };
  static float var_ImGuiCol_FrameBg[4] = { 0.20f, 0.22f, 0.27f, 1.00f };
  static float var_ImGuiCol_FrameBgHovered[4] = { 0.92f, 0.18f, 0.29f, 0.78f };
  static float var_ImGuiCol_FrameBgActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_TitleBg[4] = { 0.20f, 0.22f, 0.27f, 1.00f };
  static float var_ImGuiCol_TitleBgCollapsed[4] = { 0.20f, 0.22f, 0.27f, 0.75f };
  static float var_ImGuiCol_TitleBgActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_MenuBarBg[4] = { 0.20f, 0.22f, 0.27f, 0.47f };
  static float var_ImGuiCol_ScrollbarBg[4] = { 0.20f, 0.22f, 0.27f, 1.00f };
  static float var_ImGuiCol_ScrollbarGrab[4] = { 0.09f, 0.15f, 0.16f, 1.00f };
  static float var_ImGuiCol_ScrollbarGrabHovered[4] = { 0.92f, 0.18f, 0.29f, 0.78f };
  static float var_ImGuiCol_ScrollbarGrabActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_CheckMark[4] = { 0.71f, 0.22f, 0.27f, 1.00f };
  static float var_ImGuiCol_SliderGrab[4] = { 0.47f, 0.77f, 0.83f, 0.14f };
  static float var_ImGuiCol_SliderGrabActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_Button[4] = { 0.47f, 0.77f, 0.83f, 0.14f };
  static float var_ImGuiCol_ButtonHovered[4] = { 0.92f, 0.18f, 0.29f, 0.86f };
  static float var_ImGuiCol_ButtonActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_Header[4] = { 0.92f, 0.18f, 0.29f, 0.76f };
  static float var_ImGuiCol_HeaderHovered[4] = { 0.92f, 0.18f, 0.29f, 0.86f };
  static float var_ImGuiCol_HeaderActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_Separator[4] = { 0.14f, 0.16f, 0.19f, 1.00f };
  static float var_ImGuiCol_SeparatorHovered[4] = { 0.92f, 0.18f, 0.29f, 0.78f };
  static float var_ImGuiCol_SeparatorActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_ResizeGrip[4] = { 0.47f, 0.77f, 0.83f, 0.04f };
  static float var_ImGuiCol_ResizeGripHovered[4] = { 0.92f, 0.18f, 0.29f, 0.78f };
  static float var_ImGuiCol_ResizeGripActive[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_PlotLines[4] = { 0.86f, 0.93f, 0.89f, 0.63f };
  static float var_ImGuiCol_PlotLinesHovered[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_PlotHistogram[4] = { 0.86f, 0.93f, 0.89f, 0.63f };
  static float var_ImGuiCol_PlotHistogramHovered[4] = { 0.92f, 0.18f, 0.29f, 1.00f };
  static float var_ImGuiCol_PopupBg[4] = { 0.20f, 0.22f, 0.27f, 0.9f };
  static float var_ImGuiCol_TextSelectedBg[4] = { 0.92f, 0.18f, 0.29f, 0.43f };

  ImGui::ColorEdit4("ImGuiCol_Text##var_ImGuiCol_Text", var_ImGuiCol_Text);
  ImGui::ColorEdit4("ImGuiCol_TextDisabled##var_ImGuiCol_TextDisabled", var_ImGuiCol_TextDisabled);
  ImGui::ColorEdit4("ImGuiCol_WindowBg##var_ImGuiCol_WindowBg", var_ImGuiCol_WindowBg);
  ImGui::ColorEdit4("ImGuiCol_Border##var_ImGuiCol_Border", var_ImGuiCol_Border);
  ImGui::ColorEdit4("ImGuiCol_BorderShadow##var_ImGuiCol_BorderShadow", var_ImGuiCol_BorderShadow);
  ImGui::ColorEdit4("ImGuiCol_FrameBg##var_ImGuiCol_FrameBg", var_ImGuiCol_FrameBg);
  ImGui::ColorEdit4("ImGuiCol_FrameBgHovered##var_ImGuiCol_FrameBgHovered", var_ImGuiCol_FrameBgHovered);
  ImGui::ColorEdit4("ImGuiCol_FrameBgActive##var_ImGuiCol_FrameBgActive", var_ImGuiCol_FrameBgActive);
  ImGui::ColorEdit4("ImGuiCol_TitleBg##var_ImGuiCol_TitleBg", var_ImGuiCol_TitleBg);
  ImGui::ColorEdit4("ImGuiCol_TitleBgCollapsed##var_ImGuiCol_TitleBgCollapsed", var_ImGuiCol_TitleBgCollapsed);
  ImGui::ColorEdit4("ImGuiCol_TitleBgActive##var_ImGuiCol_TitleBgActive", var_ImGuiCol_TitleBgActive);
  ImGui::ColorEdit4("ImGuiCol_MenuBarBg##var_ImGuiCol_MenuBarBg", var_ImGuiCol_MenuBarBg);
  ImGui::ColorEdit4("ImGuiCol_ScrollbarBg##var_ImGuiCol_ScrollbarBg", var_ImGuiCol_ScrollbarBg);
  ImGui::ColorEdit4("ImGuiCol_ScrollbarGrab##var_ImGuiCol_ScrollbarGrab", var_ImGuiCol_ScrollbarGrab);
  ImGui::ColorEdit4("ImGuiCol_ScrollbarGrabHovered##var_ImGuiCol_ScrollbarGrabHovered", var_ImGuiCol_ScrollbarGrabHovered);
  ImGui::ColorEdit4("ImGuiCol_ScrollbarGrabActive##var_ImGuiCol_ScrollbarGrabActive", var_ImGuiCol_ScrollbarGrabActive);
  ImGui::ColorEdit4("ImGuiCol_CheckMark##var_ImGuiCol_CheckMark", var_ImGuiCol_CheckMark);
  ImGui::ColorEdit4("ImGuiCol_SliderGrab##var_ImGuiCol_SliderGrab", var_ImGuiCol_SliderGrab);
  ImGui::ColorEdit4("ImGuiCol_SliderGrabActive##var_ImGuiCol_SliderGrabActive", var_ImGuiCol_SliderGrabActive);
  ImGui::ColorEdit4("ImGuiCol_Button##var_ImGuiCol_Button", var_ImGuiCol_Button);
  ImGui::ColorEdit4("ImGuiCol_ButtonHovered##var_ImGuiCol_ButtonHovered", var_ImGuiCol_ButtonHovered);
  ImGui::ColorEdit4("ImGuiCol_ButtonActive##var_ImGuiCol_ButtonActive", var_ImGuiCol_ButtonActive);
  ImGui::ColorEdit4("ImGuiCol_Header##var_ImGuiCol_Header", var_ImGuiCol_Header);
  ImGui::ColorEdit4("ImGuiCol_HeaderHovered##var_ImGuiCol_HeaderHovered", var_ImGuiCol_HeaderHovered);
  ImGui::ColorEdit4("ImGuiCol_HeaderActive##var_ImGuiCol_HeaderActive", var_ImGuiCol_HeaderActive);
  ImGui::ColorEdit4("ImGuiCol_Separator##var_ImGuiCol_Separator", var_ImGuiCol_Separator);
  ImGui::ColorEdit4("ImGuiCol_SeparatorHovered##var_ImGuiCol_SeparatorHovered", var_ImGuiCol_SeparatorHovered);
  ImGui::ColorEdit4("ImGuiCol_SeparatorActive##var_ImGuiCol_SeparatorActive", var_ImGuiCol_SeparatorActive);
  ImGui::ColorEdit4("ImGuiCol_ResizeGrip##var_ImGuiCol_ResizeGrip", var_ImGuiCol_ResizeGrip);
  ImGui::ColorEdit4("ImGuiCol_ResizeGripHovered##var_ImGuiCol_ResizeGripHovered", var_ImGuiCol_ResizeGripHovered);
  ImGui::ColorEdit4("ImGuiCol_ResizeGripActive##var_ImGuiCol_ResizeGripActive", var_ImGuiCol_ResizeGripActive);
  ImGui::ColorEdit4("ImGuiCol_PlotLines##var_ImGuiCol_PlotLines", var_ImGuiCol_PlotLines);
  ImGui::ColorEdit4("ImGuiCol_PlotLinesHovered##var_ImGuiCol_PlotLinesHovered", var_ImGuiCol_PlotLinesHovered);
  ImGui::ColorEdit4("ImGuiCol_PlotHistogram##var_ImGuiCol_PlotHistogram", var_ImGuiCol_PlotHistogram);
  ImGui::ColorEdit4("ImGuiCol_PlotHistogramHovered##var_ImGuiCol_PlotHistogramHovered", var_ImGuiCol_PlotHistogramHovered);
  ImGui::ColorEdit4("ImGuiCol_PopupBg##var_ImGuiCol_PopupBg", var_ImGuiCol_PopupBg);
  ImGui::ColorEdit4("ImGuiCol_TextSelectedBg##var_ImGuiCol_TextSelectedBg", var_ImGuiCol_TextSelectedBg);

  auto& colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = *(ImVec4*)(var_ImGuiCol_Text);
  colors[ImGuiCol_TextDisabled] = *(ImVec4*)var_ImGuiCol_TextDisabled;
  colors[ImGuiCol_WindowBg] = *(ImVec4*)var_ImGuiCol_WindowBg;
  colors[ImGuiCol_Border] = *(ImVec4*)var_ImGuiCol_Border;
  colors[ImGuiCol_BorderShadow] = *(ImVec4*)var_ImGuiCol_BorderShadow;
  colors[ImGuiCol_FrameBg] = *(ImVec4*)var_ImGuiCol_FrameBg;
  colors[ImGuiCol_FrameBgHovered] = *(ImVec4*)var_ImGuiCol_FrameBgHovered;
  colors[ImGuiCol_FrameBgActive] = *(ImVec4*)var_ImGuiCol_FrameBgActive;
  colors[ImGuiCol_TitleBg] = *(ImVec4*)var_ImGuiCol_TitleBg;
  colors[ImGuiCol_TitleBgCollapsed] = *(ImVec4*)var_ImGuiCol_TitleBgCollapsed;
  colors[ImGuiCol_TitleBgActive] = *(ImVec4*)var_ImGuiCol_TitleBgActive;
  colors[ImGuiCol_MenuBarBg] = *(ImVec4*)var_ImGuiCol_MenuBarBg;
  colors[ImGuiCol_ScrollbarBg] = *(ImVec4*)var_ImGuiCol_ScrollbarBg;
  colors[ImGuiCol_ScrollbarGrab] = *(ImVec4*)var_ImGuiCol_ScrollbarGrab;
  colors[ImGuiCol_ScrollbarGrabHovered] = *(ImVec4*)var_ImGuiCol_ScrollbarGrabHovered;
  colors[ImGuiCol_ScrollbarGrabActive] = *(ImVec4*)var_ImGuiCol_ScrollbarGrabActive;
  colors[ImGuiCol_CheckMark] = *(ImVec4*)var_ImGuiCol_CheckMark;
  colors[ImGuiCol_SliderGrab] = *(ImVec4*)var_ImGuiCol_SliderGrab;
  colors[ImGuiCol_SliderGrabActive] = *(ImVec4*)var_ImGuiCol_SliderGrabActive;
  colors[ImGuiCol_Button] = *(ImVec4*)var_ImGuiCol_Button;
  colors[ImGuiCol_ButtonHovered] = *(ImVec4*)var_ImGuiCol_ButtonHovered;
  colors[ImGuiCol_ButtonActive] = *(ImVec4*)var_ImGuiCol_ButtonActive;
  colors[ImGuiCol_Header] = *(ImVec4*)var_ImGuiCol_Header;
  colors[ImGuiCol_HeaderHovered] = *(ImVec4*)var_ImGuiCol_HeaderHovered;
  colors[ImGuiCol_HeaderActive] = *(ImVec4*)var_ImGuiCol_HeaderActive;
  colors[ImGuiCol_Separator] = *(ImVec4*)var_ImGuiCol_Separator;
  colors[ImGuiCol_SeparatorHovered] = *(ImVec4*)var_ImGuiCol_SeparatorHovered;
  colors[ImGuiCol_SeparatorActive] = *(ImVec4*)var_ImGuiCol_SeparatorActive;
  colors[ImGuiCol_ResizeGrip] = *(ImVec4*)var_ImGuiCol_ResizeGrip;
  colors[ImGuiCol_ResizeGripHovered] = *(ImVec4*)var_ImGuiCol_ResizeGripHovered;
  colors[ImGuiCol_ResizeGripActive] = *(ImVec4*)var_ImGuiCol_ResizeGripActive;
  colors[ImGuiCol_PlotLines] = *(ImVec4*)var_ImGuiCol_PlotLines;
  colors[ImGuiCol_PlotLinesHovered] = *(ImVec4*)var_ImGuiCol_PlotLinesHovered;
  colors[ImGuiCol_PlotHistogram] = *(ImVec4*)var_ImGuiCol_PlotHistogram;
  colors[ImGuiCol_PlotHistogramHovered] = *(ImVec4*)var_ImGuiCol_PlotHistogramHovered;
  colors[ImGuiCol_PopupBg] = *(ImVec4*)var_ImGuiCol_PopupBg;
  colors[ImGuiCol_TextSelectedBg] = *(ImVec4*)var_ImGuiCol_TextSelectedBg;

  ImGui::End();
}

} // namespace game2d