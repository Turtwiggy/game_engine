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

void
game2d::update_ui_profiler_system(entt::registry& registry)
{
#ifdef _DEBUG
  // less than X-fps?! what is this?!
  // if (ImGui::GetIO().Framerate <= 45 && ImGui::GetFrameCount() > 120)
  //   std::cout << "(profiler) fps drop?!" << std::endl;
#endif

  auto& p = registry.ctx().at<Profiler>();

  // Profiler
  ImGui::Begin("Profiler ", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
    ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
    ImGui::Text("Draw Calls QR %i", engine::quad_renderer::QuadRenderer::draw_calls());
    ImGui::Text("Draw Calls TR %i", engine::triangle_renderer::TriangleRenderer::draw_calls());
    ImGui::Text("Draw Calls TFR %i", engine::triangle_fan_renderer::TriangleFanRenderer::draw_calls());
    ImGui::Separator();
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);

    for (int i = 0; i < p.update_results.size(); i++) {
      const auto& result = p.update_results[i];
      ImGui::Text("update: %s %f", result.name.c_str(), result.ms);
    };

    for (int i = 0; i < p.fixed_update_results.size(); i++) {
      const auto& result = p.fixed_update_results[i];
      ImGui::Text("fixed_update: %s %f", result.name.c_str(), result.ms);
    };

    p.update_results.clear();
    p.fixed_update_results.clear();
  }
  ImGui::End();
};