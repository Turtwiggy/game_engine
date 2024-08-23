#include "system.hpp"

#include "actors/base.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/ux_hoverable/components.hpp"

#include <magic_enum.hpp>

namespace game2d {

void
update_ux_selectable_by_keyboard_system(entt::registry& r)
{
  // Clear anything thats hovered
  // cleared by ux_hoverable() system

  // Clear anything that's selected
  {
    const auto& view = r.view<const SelectedComponent>();
    r.remove<SelectedComponent>(view.begin(), view.end());
  }

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  static int chosen_idx = 0;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_Z))
    chosen_idx--;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_X))
    chosen_idx++;

  const auto& view = r.view<PlayerComponent>();

  // limit selectables
  chosen_idx = glm::max(chosen_idx, 0);
  chosen_idx = glm::min(chosen_idx, int(view.size() - 1));

  for (int i = 0; const auto& [e, player_c] : view.each()) {
    const auto name = std::string(magic_enum::enum_name(r.get<EntityTypeComponent>(e).type));
    if (chosen_idx == i)
      r.emplace_or_replace<SelectedComponent>(e);
    i++;
  }
}

} // namespace game2d