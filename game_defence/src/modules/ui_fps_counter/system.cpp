#include "system.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_fps_counter_system(entt::registry& r)
{
  const bool show_fps_counter = true;
  if (show_fps_counter) {
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoDecoration;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoDocking;
    flags |= ImGuiWindowFlags_NoSavedSettings;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0, 0 });
    ImGui::Begin("FPS", NULL, flags);
    ImGui::Text("FPS: %0.2f ", ImGui::GetIO().Framerate);
    ImGui::End();
  }
}

} // namespace game2d