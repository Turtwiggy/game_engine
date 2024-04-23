#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"
#include <glm/glm.hpp>

namespace game2d {

void
update_spaceship_door_system(entt::registry& r, const float& dt)
{
  //
  // PressurePlate System
  // Resolve Collisions
  //
  std::vector<entt::entity> doors_to_open;
  std::vector<entt::entity> doors_to_close;
  {
    const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
    for (const Collision2D& coll : physics.collision_stay) {
      const auto a = static_cast<entt::entity>(coll.ent_id_0);
      const auto b = static_cast<entt::entity>(coll.ent_id_1);

      const auto [a_ent, b_ent] = collision_of_interest<CursorComponent, SpaceshipPressureplateComponent>(r, a, b);
      if (a_ent != entt::null && b_ent != entt::null) {
        // const auto& cursor = a_ent;
        const auto& plate = b_ent;

        const auto& plate_comp = r.get<SpaceshipPressureplateComponent>(plate);
        if (plate_comp.type == PressurePlateType::OPEN)
          doors_to_open.push_back(plate_comp.door);
        if (plate_comp.type == PressurePlateType::CLOSE)
          doors_to_close.push_back(plate_comp.door);
      }
    }
  }

  ImGui::Begin("System_DoorDebug");

  const auto& view = r.view<SpaceshipDoorComponent, AABB>();
  for (const auto& [e, door, door_aabb] : view.each()) {
    ImGui::Separator();
    auto& door_state = door.state;
    const auto door_state_name = std::string(magic_enum::enum_name(door_state));
    ImGui::Text("DoorState: %s", door_state_name.c_str());

    // Process state until it becomes open or closed
    const auto& closed_size = door.closed_size;
    const bool is_horizontal = closed_size.x > closed_size.y;
    const int closing_speed = 50;
    const float amount = closing_speed * dt;

    // by opening, reduce size of door
    //
    const bool open_this_door = std::find(doors_to_open.begin(), doors_to_open.end(), e) != doors_to_open.end();
    if (open_this_door) {
      ImGui::Text("Opening...!");
      door.state = SpaceshipDoorState::BETWIXT;

      if (is_horizontal)
        door.to_close_increment.x -= amount;
      else
        door.to_close_increment.y -= amount;
    }

    // by closing, increase size of door
    //
    const bool close_this_door = std::find(doors_to_close.begin(), doors_to_close.end(), e) != doors_to_close.end();
    if (close_this_door) {
      ImGui::Text("Closing...!");
      door.state = SpaceshipDoorState::BETWIXT;

      if (is_horizontal)
        door.to_close_increment.x += amount;
      else
        door.to_close_increment.y += amount;
    }

    // Update the door size
    //
    {
      const auto store_float_overflow = [](float& amount) -> int {
        const int amount_int = glm::round(amount);
        amount -= amount_int;
        return amount_int;
      };
      const int x_change = store_float_overflow(door.to_close_increment.x);
      const int y_change = store_float_overflow(door.to_close_increment.y);

      // Set the initial width of the door
      if (glm::abs(x_change) > 0 && close_this_door && is_horizontal)
        door_aabb.size.y = closed_size.y;
      else if (glm::abs(y_change) > 0 && close_this_door && !is_horizontal)
        door_aabb.size.x = closed_size.x;

      set_size(r, e, { door_aabb.size.x + x_change, door_aabb.size.y + y_change });
    }

    // Check if door is fully open
    //
    if (door_aabb.size.x <= 0.0f || door_aabb.size.y <= 0.0f) {
      door.state = SpaceshipDoorState::OPEN;
      set_size(r, e, { 0, 0 });

      // If fully open, remove solid component
      if (const auto* solid = r.try_get<PhysicsSolidComponent>(e))
        r.remove<PhysicsSolidComponent>(e);
    } else
      r.emplace_or_replace<PhysicsSolidComponent>(e);

    // Check if door is open
    //
    if (door_aabb.size.x >= closed_size.x && door_aabb.size.y >= closed_size.y) {
      door.state = SpaceshipDoorState::CLOSED;
      set_size(r, e, { closed_size.x, closed_size.y });
    }

    // For fun, calculate percentage door is open/closed?
    //
    float percent_closed = 0.0f;
    if (is_horizontal)
      percent_closed = door_aabb.size.x / closed_size.x;
    else
      percent_closed = door_aabb.size.y / closed_size.y;
    ImGui::Text("Percentage Closed: %f", percent_closed * 100.0f);
  }

  ImGui::End();
}

} // namespace game2d