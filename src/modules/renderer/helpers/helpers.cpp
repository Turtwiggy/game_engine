// header
#include "modules/renderer/helpers/helpers.hpp"

// other lib
#include <imgui.h>
#include <imgui_internal.h>

game2d::ViewportInfo
game2d::render_texture_to_imgui_viewport(const int64_t& tex_unit)
{
  static bool dockspace_open = true;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
  window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoNavFocus;
  }

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", &dockspace_open, window_flags);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  // Dockspace
  ImGuiIO& io = ImGui::GetIO();

  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("RootDockSpace");

    if (!ImGui::DockBuilderGetNode(dockspace_id)) {
      ImVec2 vps = ImGui::GetContentRegionAvail();
      ImGui::DockBuilderRemoveNode(dockspace_id);
      ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockspace_id, vps);

      ImGuiID dock_id_main = dockspace_id;
      // ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Down, 0.15f, nullptr, &dock_id_main);
      ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.15f, nullptr, &dock_id_main);
      ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.15f, nullptr, &dock_id_main);
      ImGuiID dock_id_right_bottom =
        ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right);
      ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Up, 0.15f, nullptr, &dock_id_main);

      // TODO: improve this
      ImGui::DockBuilderDockWindow("Viewport", dock_id_main);
      // ImGui::DockBuilderDockWindow("Profiler", dock_id_left);
      // ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
      ImGui::DockBuilderDockWindow("Player", dock_id_left);
      ImGui::DockBuilderDockWindow("Game Events", dock_id_top);
      // ImGui::DockBuilderDockWindow("Physics", dock_id_right);

      ImGui::DockBuilderFinish(dock_id_main);
    }
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGuiWindowFlags viewport_flags = ImGuiWindowFlags_NoTitleBar;

  ImGuiWindowClass window_class;
  window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
  ImGui::SetNextWindowClass(&window_class);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("Viewport", NULL, viewport_flags);
  ImGui::PopStyleVar();
  ImVec2 viewport_size = ImGui::GetContentRegionAvail();

  ViewportInfo vi;
  vi.focused = ImGui::IsWindowFocused();
  vi.hovered = ImGui::IsWindowHovered();
  vi.size = viewport_size;
  vi.pos = ImGui::GetWindowPos();

  // render opengl texture to imgui
  ImGui::Image((ImTextureID)tex_unit, viewport_size, ImVec2(0, 0), ImVec2(1, 1));

  ImGui::End();

  ImGui::End();

  return vi;
}
