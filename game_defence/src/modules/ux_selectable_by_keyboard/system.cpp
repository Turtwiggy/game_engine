#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/ux_hoverable/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

namespace game2d {

void
update_ux_selectable_by_keyboard_system(entt::registry& r)
{
  // Clear anything thats hovered
  {
    const auto& view = r.view<HoveredComponent>();
    r.remove<HoveredComponent>(view.begin(), view.end());
  }
  // Clear anything that's selected
  {
    const auto& view = r.view<SelectedComponent>();
    r.remove<SelectedComponent>(view.begin(), view.end());
  }

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  static int chosen_idx = 0;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_Q))
    chosen_idx--;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_E))
    chosen_idx++;

  ImGui::Begin("Debug__UXHoverableByKeyboard");

  const auto& view = r.view<PlayerComponent>();

  // int size = 0;
  // for (const auto& [e, tc, pc] : view.each())
  //   size++;

  // limit selectables
  chosen_idx = glm::max(chosen_idx, 0);
  chosen_idx = glm::min(chosen_idx, int(view.size() - 1));
  ImGui::Text("ChosenIdx: %i", chosen_idx);

  for (int i = 0; const auto& [e, player_c] : view.each()) {
    ImGui::PushID(static_cast<uint32_t>(e));

    const auto name = std::string(magic_enum::enum_name(r.get<EntityTypeComponent>(e).type));
    if (chosen_idx == i) {
      r.emplace_or_replace<SelectedComponent>(e);
      ImGui::Text("name %s (SELECTED)", name.c_str());
    } else
      ImGui::Text("name %s", name.c_str());

    i++;
    ImGui::PopID();
  }

  ImGui::End();
}

} // namespace game2d