#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/turret/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include "glm/glm.hpp"
#include "imgui.h"
#include "magic_enum.hpp"

#include <string>

namespace game2d {

void
update_ui_economy_system(entt::registry& r)
{
  const auto& econ = get_first_component<SINGLETON_Economy>(r);
  const int GRID_SIZE = 16; // hmm
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r) + glm::ivec2(GRID_SIZE / 2, GRID_SIZE / 2);
  const glm::ivec2 grid_position = engine::grid::world_space_to_grid_space(mouse_position, GRID_SIZE);
  const glm::ivec2 world_position = engine::grid::grid_space_to_world_space(grid_position, GRID_SIZE);

  const std::vector<std::string> items{
    //
    std::string(magic_enum::enum_name(EntityType::actor_turret))
    //
  };

  ImGui::Begin("Economy");
  ImGui::Text("Money: %i", econ.kills);

  const auto& view = r.view<TurretComponent>();
  if (view.size() > 0) {
    const auto& first_turret = get_first_component<TurretComponent>(r);
    ImGui::Text("time_between_bullets %f", first_turret.time_between_bullets);
  }

  if (ImGui::Button("Upgrade all turrets")) {
    for (auto [entity, turret] : view.each()) {
      turret.time_between_bullets -= 0.01f;
      turret.time_between_bullets = glm::max(0.05f, turret.time_between_bullets);
    }
  }

  // Here we store our selection data as an index.
  static ImGuiComboFlags econ_flags = 0;
  static int econ_item_current_idx = 0;

  // Pass in the preview value visible before opening the combo (it could be anything)
  const char* combo_preview_value = items[econ_item_current_idx].c_str();
  if (ImGui::BeginCombo("wombocomboforecon", combo_preview_value, econ_flags)) {
    for (int n = 0; n < items.size(); n++) {
      const bool is_selected = (econ_item_current_idx == n);
      if (ImGui::Selectable(items[n].c_str(), is_selected))
        econ_item_current_idx = n;
      // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  ImGui::End();

  // Entity To place!
  EntityType type = magic_enum::enum_cast<EntityType>(items[econ_item_current_idx]).value();

  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  if (!ri.viewport_hovered)
    return;

  bool place = get_mouse_rmb_press();
  if (place) {
    CreateEntityRequest request;
    request.type = type;
    request.position = { world_position.x, world_position.y, 0 };
    r.emplace<CreateEntityRequest>(r.create(), request);
  }
};

} // namespace game2d