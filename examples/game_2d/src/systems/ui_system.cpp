// header
#include "systems/ui_system.hpp"

// components
#include "components/player.hpp"
#include "components/profiler_stats.hpp"
#include "components/sprite.hpp"
#include "components/z_index.hpp"

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
    auto view = registry.view<const Sprite>();
    ImGui::Begin("Profiler");
    ImGui::Text("Rendering entities: %i", view.size());
    ImGui::Separator();
    ImGui::Text("Physics %f", p.physics_elapsed_ms);
    ImGui::Text("Render %f", p.render_elapsed_ms);
    ImGui::Text("Frame ms total %f", 1000.0f / ImGui::GetIO().Framerate);
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

  // Top menu bar
  glm::ivec2 screen_wh = app.get_window().get_size();
  if (ImGui::BeginMainMenuBar()) {
    ImGui::Text("Warlords");

    ImGui::SameLine(screen_wh.x - 120.0f);
    {
      float framerate = ImGui::GetIO().Framerate;
      ImGui::Text("FPS %0.2f", framerate);
    }
    ImGui::SameLine(screen_wh.x - 45.0f);
    if (ImGui::Button("Quit")) {
      app.shutdown();
    }
    ImGui::EndMainMenuBar();
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