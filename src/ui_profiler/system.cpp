// header
#include "system.hpp"

// components
#include "entt/helpers.hpp"
#include "helpers.hpp"
#include "modules/physics/components.hpp"
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

  auto& profiler = get_first_component<SINGLETON_Profiler>(r);
  const auto& physics = get_first_component<const SINGLETON_PhysicsComponent>(r);
  const auto& objs = r.view<const PhysicsTransformXComponent>();
  const auto& solids = r.view<const PhysicsSolidComponent>();
  const auto& actors = r.view<const PhysicsActorComponent>();

  // Profiler
  ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
    ImGui::Text("¬¬ Renderer");
    ImGui::Text("Renderables: %i", r.view<TransformComponent>().size());
    ImGui::Text("Draw Calls QR %i", engine::quad_renderer::QuadRenderer::draw_calls());
    ImGui::Text("Draw Calls TR %i", engine::triangle_renderer::TriangleRenderer::draw_calls());
    ImGui::Text("Draw Calls TFR %i", engine::triangle_fan_renderer::TriangleFanRenderer::draw_calls());
    ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);

    ImGui::Text("¬¬ Physics");
    ImGui::Text("Objects %i", objs.size());
    ImGui::Text("Solids %i", solids.size());
    ImGui::Text("Actors %i", actors.size());
    ImGui::Text("collision_enter %i", physics.collision_enter.size());
    ImGui::Text("collision_stay %i", physics.collision_stay.size());
    ImGui::Text("collision_exit %i", physics.collision_exit.size());
    // for (const auto& p : physics.collision_stay) {
    //   EntityType e0 = registry.get<EntityTypeComponent>(static_cast<entt::entity>(p.ent_id_0)).type;
    //   EntityType e1 = registry.get<EntityTypeComponent>(static_cast<entt::entity>(p.ent_id_1)).type;
    //   ImGui::Text("CollisionStay between types %i, %i", static_cast<int>(e0), static_cast<int>(e1));
    // }

    for (const auto& result : profiler.update_results)
      ImGui::Text("update: %s %f", result.name.c_str(), result.ms);
    for (const auto& result : profiler.fixed_update_results)
      ImGui::Text("fixed_update: %s %f", result.name.c_str(), result.ms);

    profiler.update_results.clear();
    profiler.fixed_update_results.clear();
  }
  ImGui::End();
};