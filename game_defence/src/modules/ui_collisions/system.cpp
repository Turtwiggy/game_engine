#include "system.hpp"

#include "entt/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  ImGui::Begin("Collisions");

  ImGui::Text("Enter %i", physics.collision_enter.size());
  ImGui::Text("Stay %i", physics.collision_stay.size());
  ImGui::Text("Exit %i", physics.collision_exit.size());

  ImGui::End();
}

} // namespace game2d