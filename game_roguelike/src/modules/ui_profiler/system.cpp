// header
#include "system.hpp"

// components
#include "components/actors.hpp"
#include "helpers.hpp"
#include "modules/ui_profiler/components.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

// helpers
#include "renderer/helpers/batch_quad.hpp"
#include "renderer/helpers/batch_triangle.hpp"
#include "renderer/helpers/batch_triangle_fan.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_profiler_system(GameEditor& editor, Game& game)
{
#ifdef _DEBUG
  // less than X-fps?! what is this?!
  // if (ImGui::GetIO().Framerate <= 45 && ImGui::GetFrameCount() > 120)
  //   std::cout << "(profiler) fps drop?!" << "\n";
#endif

  auto& profiler = editor.profiler;
  const auto& registry = game.state;
  const auto& physics = game.physics;
  const auto& objs = registry.view<PhysicsTransformComponent>();
  const auto& solids = registry.view<PhysicsSolidComponent>();
  const auto& actors = registry.view<PhysicsActorComponent>();

  // Profiler
  ImGui::Begin("Profiler", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
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

    ImGui::Text("¬¬ Renderer");
    ImGui::Text("Draw Calls QR %i", engine::quad_renderer::QuadRenderer::draw_calls());
    ImGui::Text("Draw Calls TR %i", engine::triangle_renderer::TriangleRenderer::draw_calls());
    ImGui::Text("Draw Calls TFR %i", engine::triangle_fan_renderer::TriangleFanRenderer::draw_calls());
    ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);

    for (const auto& result : profiler.update_results)
      ImGui::Text("update: %s %f", result.name.c_str(), result.ms);
    for (const auto& result : profiler.fixed_update_results)
      ImGui::Text("fixed_update: %s %f", result.name.c_str(), result.ms);

    profiler.update_results.clear();
    profiler.fixed_update_results.clear();
  }
  ImGui::End();
};