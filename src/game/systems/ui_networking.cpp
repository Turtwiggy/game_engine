#include "ui_place_entity.hpp"

#include "engine/maths/grid.hpp"
#include "game/create_entities.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/renderer/components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_networking_system(entt::registry& r)
{
#ifdef _DEBUG
  bool show_imgui_demo_window = true;
  ImGui::ShowDemoWindow(&show_imgui_demo_window);
  // less than X-fps?! what is this?!
  if (ImGui::GetIO().Framerate <= 45 && ImGui::GetFrameCount() > 120)
    std::cout << "(profiler) fps drop?!" << std::endl;
#endif

  ImGui::Begin("Networking...");
  ImGui::Text("Hmm");
  ImGui::End();
}

} // namespace game2d