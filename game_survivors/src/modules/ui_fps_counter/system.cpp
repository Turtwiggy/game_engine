#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "imgui.h"
#include "modules/renderer/components.hpp"

namespace game2d {

void
update_ui_fps_counter_system(entt::registry& r)
{
  const bool show_fps_counter = true;
  if (show_fps_counter) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    static auto pos = glm::vec2{ 0, ri.viewport_size_render_at.y - (ImGui::GetFontSize() * 2.0f) }; // bl

    // ImGui::Begin("DebugFps");
    // imgui_draw_vec2("fps_pos", pos);
    // ImGui::End();

    ImGui::SetNextWindowPos(ImVec2{ pos.x, pos.y }, ImGuiCond_Always, { 0, 0 });

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoDecoration;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoDocking;
    flags |= ImGuiWindowFlags_NoSavedSettings;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    flags |= ImGuiWindowFlags_NoInputs;
    flags |= ImGuiWindowFlags_AlwaysAutoResize;

    const ImVec2 pivot = { 0.0f, 0.5f };
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);

    ImGui::Begin("FPS", NULL, flags);
    ImGui::Text("%d FPS", (int)ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::PopStyleVar();
  }
}

} // namespace game2d