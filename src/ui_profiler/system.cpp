// header
#include "system.hpp"

// components
#include "entt/helpers.hpp"
#include "helpers.hpp"
#include "renderer/components.hpp"
#include "ui_profiler/components.hpp"

// helpers
#include "renderer/helpers/batch_quad.hpp"
#include "renderer/helpers/batch_triangle.hpp"
#include "renderer/helpers/batch_triangle_fan.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_profiler_system(entt::registry& r)
{
  // #ifdef _DEBUG
  //   // less than X-fps?! what is this?!
  //   if (ImGui::GetIO().Framerate <= 45 && ImGui::GetFrameCount() > 120)
  //     std::cout << "(profiler) fps drop?! \n";
  // #endif

  const auto& registry = r;
  auto& profiler = get_first_component<Profiler>(r);

  // Profiler
  ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
    ImGui::Text("¬¬ Physics");
    // box2d

    ImGui::Text("¬¬ Renderer");
    ImGui::Text("Draw Calls QR %i", engine::quad_renderer::QuadRenderer::draw_calls());
    ImGui::Text("Draw Calls TR %i", engine::triangle_renderer::TriangleRenderer::draw_calls());
    ImGui::Text("Draw Calls TFR %i", engine::triangle_fan_renderer::TriangleFanRenderer::draw_calls());
    ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);

    const auto& transforms = r.view<TransformComponent>();
    ImGui::Text("Transforms: %i", transforms.size());

    for (const auto& result : profiler.update_results)
      ImGui::Text("update: %s %f", result.name.c_str(), result.ms);
    for (const auto& result : profiler.fixed_update_results)
      ImGui::Text("fixed_update: %s %f", result.name.c_str(), result.ms);

    profiler.update_results.clear();
    profiler.fixed_update_results.clear();
  }
  ImGui::End();
};