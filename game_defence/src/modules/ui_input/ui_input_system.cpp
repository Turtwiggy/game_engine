#include "ui_input_system.hpp"

#include "modules/actor_player/components.hpp"

#include "imgui.h"
#include <format>

namespace game2d {

void
update_ui_input_system(entt::registry& r)
{
  ImGuiWindowFlags flags = 0;

  ImGui::Begin("PlayerInput", NULL, flags);

  const auto& view = r.view<const PlayerComponent, const InputComponent>();
  for (const auto& [e, player_c, input_c] : view.each()) {
    ImGui::PushID(static_cast<uint32_t>(e));

    ImGui::Text("%s", std::format("lx: {} ly: {}", input_c.lx, input_c.ly).c_str());
    ImGui::Text("%s", std::format("rx: {} ry: {}", input_c.rx, input_c.ry).c_str());

    ImGui::PopID();
  }

  ImGui::End();
}

} // namespace game2d