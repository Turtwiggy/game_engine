#include "ui_input_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/actor_player/components.hpp"

#include "imgui.h"
#include <SDL_keyboard.h>
#include <format>

namespace game2d {

void
update_ui_input_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);

  static std::vector<InputEvent> display_inputs;

  ImGuiWindowFlags flags = 0;

  ImGui::Begin("PlayerInput", NULL, flags);
  ImGui::Text("mouse_pos: %i %i", mouse_pos.x, mouse_pos.y);

  const auto& view = r.view<const PlayerComponent, const InputComponent>();
  for (const auto& [e, player_c, input_c] : view.each()) {
    ImGui::PushID(static_cast<uint32_t>(e));

    ImGui::Text("%s", std::format("lx: {} ly: {}", input_c.lx, input_c.ly).c_str());
    ImGui::Text("%s", std::format("rx: {} ry: {}", input_c.rx, input_c.ry).c_str());

    ImGui::PopID();
  }

  for (const auto& i : input_c.unprocessed_inputs) {
    ImGui::Text("%s ", std::format("{}", engine::convert_enum_to_string(i.type)).c_str());
    ImGui::SameLine();
    ImGui::Text("%s", std::format("mouse: {}", i.mouse).c_str());
    ImGui::Text("%s", std::format("kb: {}", SDL_GetScancodeName(i.keyboard)).c_str());
    ImGui::SameLine();
    ImGui::Text("%s ", std::format("{}", engine::convert_enum_to_string(i.state)).c_str());
  }

  ImGui::End();
}

} // namespace game2d