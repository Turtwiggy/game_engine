// header
#include "systems/ui_system.hpp"

// components
#include "components/player.hpp"
#include "components/position.hpp"
#include "components/profiler_stats.hpp"
#include "components/z_index.hpp"

// helpers
#include "helpers/renderers/batch_quad.hpp"
#include "helpers/renderers/batch_triangle.hpp"
#include "helpers/renderers/batch_triangle_fan.hpp"

// other lib headers
#include <imgui.h>

void
game2d::init_ui_system(entt::registry& registry)
{
  ProfilerStats p;
  registry.set<ProfilerStats>(p);
}

void
game2d::update_ui_system(entt::registry& registry, engine::Application& app)
{
  // bool show_imgui_demo_window = false;
  // ImGui::ShowDemoWindow(&show_imgui_demo_window);
  ProfilerStats& p = registry.ctx<ProfilerStats>();

  // Profiler
  {
    int pos_ints = 0;
    {
      auto view = registry.view<const PositionInt>();
      pos_ints = view.size();
    }
    auto view = registry.view<const PositionFloat>();
    int pos_floats = view.size();

    ImGui::Begin("Profiler");
    ImGui::Text("FPS %f", ImGui::GetIO().Framerate);
    ImGui::Text("Rendering entities: %i", pos_ints + pos_floats);
    ImGui::Separator();
    ImGui::Text("Physics %f", p.physics_elapsed_ms);
    ImGui::Text("Input %f", p.input_elapsed_ms);
    ImGui::Text("Game Tick %f", p.game_tick_elapsed_ms);
    ImGui::Text("Render %f", p.render_elapsed_ms);
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Draw Calls QR %i", engine::quad_renderer::QuadRenderer::draw_calls());
    ImGui::Text("Draw Calls TR %i", engine::triangle_renderer::TriangleRenderer::draw_calls());
    ImGui::Text("Draw Calls TFR %i", engine::triangle_fan_renderer::TriangleFanRenderer::draw_calls());
    ImGui::End();
  }

  // Player Info
  {
    // auto view = registry.view<const Player, const ZIndex>();
    // ImGui::Begin("Player");
    // view.each([](const auto entity, const auto& player, const auto& z) {
    //   //
    //   ImGui::Text("Player z-index: %i", z.index);
    // });
    // ImGui::End();
  }
}

// static std::function<void()> ui_limit_framerate_callback;
// static std::function<void()> ui_vsync_toggled_callback;
//     ImGui::Separator();
//     ImGui::Text("Settings");
//     bool temp;
//     { // limit framerate
//       temp = ui_limit_framerate;
//       ImGui::Checkbox("Limit Framerate", &temp);
//       if (temp != ui_limit_framerate) {
//         std::cout << "Limit fps toggled to: " << temp << std::endl;
//         app.limit_fps = temp;
//       }
//       ui_limit_framerate = temp;
//     }
//     { // use vsync
//       temp = ui_use_vsync;
//       ImGui::Checkbox("VSync", &temp);
//       if (temp != ui_use_vsync) {
//         std::cout << "vsync toggled to: " << temp << std::endl;
//         app.get_window().set_vsync_opengl(temp);
//       }
//       ui_use_vsync = temp;
//     }
//     { // toggle fullsceren
//       temp = ui_fullscreen;
//       ImGui::Checkbox("Fullscreen", &temp);
//       if (temp != ui_fullscreen) {
//         std::cout << "ui_fullscreen toggled to: " << temp << std::endl;
//         toggle_fullscreen(app, screen_wh, projection, resources.tex_id_lighting, resources.tex_id_main_scene);
//         gs.point_lights.clear();
//         add_lighting(gs, screen_wh);
//       }
//       ui_fullscreen = temp;
//     }