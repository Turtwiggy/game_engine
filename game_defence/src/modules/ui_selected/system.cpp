#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
update_ui_selected(entt::registry& r)
{
  auto& ui = get_first_component<SINGLE_SelectedUI>(r);

  const auto& view = r.view<SelectedComponent, TransformComponent>();
  const int selected = view.size_hint();

  // create ui
  for (int i = selected; i > ui.instantiated_ui.size(); i--) {
    const auto new_ui = create_gameplay(r, EntityType::empty_with_transform);

    // set outline sprite
    set_sprite(r, new_ui, "CURSOR_1");

    r.emplace<SelectedUIComponent>(new_ui);
    ui.instantiated_ui.push_back(new_ui);
  }

  // destroy ui
  for (auto i = ui.instantiated_ui.size(); i > selected; i--) {
    const auto idx = i - 1;
    auto entity = ui.instantiated_ui[idx];
    r.destroy(entity); // fine because no aabb
    ui.instantiated_ui.erase(ui.instantiated_ui.begin() + idx);
  }

  // get all positions from selected entities
  std::vector<glm::ivec3> positions;
  for (const auto& [entity, selected, transform] : view.each())
    positions.push_back(transform.position);

  // update positions
  const auto& ui_view = r.view<SelectedUIComponent, TransformComponent>();
  for (int i = 0; const auto& [e, instance, transform] : ui_view.each()) {
    transform.position = positions[i];
    i++;
  }
}

}; // namespace game2d