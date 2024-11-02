#include "ui_combat_designer_helpers.hpp"

#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"

namespace game2d {

void
activate_unit(entt::registry& r, entt::entity e)
{
  // stop the camera moving freely, and focus on the unit
  const auto view = r.view<CameraFreeMove>();
  r.destroy(view.begin(), view.end());

  // Update the selected unit/camera to the new unit
  const auto& selected_view = r.view<SelectedComponent>();
  r.remove<SelectedComponent>(selected_view.begin(), selected_view.end());
  r.emplace<SelectedComponent>(e);

  // move camera to newly selected unit
  const auto& follow_view = r.view<CameraLerpToTarget>();
  r.remove<CameraLerpToTarget>(follow_view.begin(), follow_view.end());
  r.emplace<CameraLerpToTarget>(e);

  const auto& input_view = r.view<InputComponent>();
  r.remove<InputComponent>(input_view.begin(), input_view.end());
  r.emplace<InputComponent>(e);

  const auto& keyboard_view = r.view<KeyboardComponent>();
  r.remove<KeyboardComponent>(keyboard_view.begin(), keyboard_view.end());
  r.emplace<KeyboardComponent>(e);
}

} // namespace game2d