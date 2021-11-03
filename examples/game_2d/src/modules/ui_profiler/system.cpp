// header
#include "system.hpp"

// components
#include "modules/renderer/components.hpp"
#include "modules/ui_profiler/components.hpp"

// helpers
#include "modules/renderer/helpers/renderers/batch_quad.hpp"
#include "modules/renderer/helpers/renderers/batch_triangle.hpp"
#include "modules/renderer/helpers/renderers/batch_triangle_fan.hpp"

// other lib headers
#include <imgui.h>

void
game2d::init_ui_profiler_system(entt::registry& registry)
{
  registry.set<Profiler>(Profiler());
}

void
game2d::update_ui_profiler_system(entt::registry& registry, engine::Application& app)
{
  // bool show_imgui_demo_window = true;
  // ImGui::ShowDemoWindow(&show_imgui_demo_window);

  Profiler& p = registry.ctx<Profiler>();

  // Profiler
  ImGui::Begin("Profiler ", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
    ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Physics %f", p.physics_elapsed_ms);
    ImGui::Text("Game Tick %f", p.game_tick_elapsed_ms);
    ImGui::Text("Render %f", p.render_elapsed_ms);
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Draw Calls QR %i", engine::quad_renderer::QuadRenderer::draw_calls());
    ImGui::Text("Draw Calls TR %i", engine::triangle_renderer::TriangleRenderer::draw_calls());
    ImGui::Text("Draw Calls TFR %i", engine::triangle_fan_renderer::TriangleFanRenderer::draw_calls());
  }
  ImGui::End();
};