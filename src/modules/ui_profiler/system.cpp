// header
#include "system.hpp"

// components
#include "modules/renderer/components.hpp"
#include "modules/ui_profiler/components.hpp"

// helpers
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/helpers/batch_triangle.hpp"
#include "modules/renderer/helpers/batch_triangle_fan.hpp"

// other lib headers
#include <imgui.h>

// stdlib
#include <iostream>

void
game2d::update_ui_profiler_system(entt::registry& registry)
{
#ifdef _DEBUG
  // bool show_imgui_demo_window = false;
  // ImGui::ShowDemoWindow(&show_imgui_demo_window);
  // less than X-fps?! what is this?!
  // if (ImGui::GetIO().Framerate <= 45 && ImGui::GetFrameCount() > 120)
  //   std::cout << "(profiler) fps drop?!" << std::endl;
#endif

  Profiler& p = registry.ctx().at<Profiler>();

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