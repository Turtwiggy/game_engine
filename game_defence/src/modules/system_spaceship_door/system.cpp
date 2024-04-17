#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"
#include "magic_enum.hpp"
#include <glm/glm.hpp>

namespace game2d {

void
update_spaceship_door_system(entt::registry& r, const float& dt)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  const auto open_hovered_door_key = SDL_SCANCODE_O;
  const auto close_hovered_door_key = SDL_SCANCODE_P;
  const bool open = get_key_held(input, open_hovered_door_key);
  const bool close = get_key_held(input, close_hovered_door_key);

  ImGui::Begin("System_DoorDebug");

  const auto& view = r.view<SpaceshipDoorComponent, AABB>();
  for (const auto& [e, door, door_aabb] : view.each()) {
    ImGui::Separator();
    auto& door_state = door.state;
    const auto door_state_name = std::string(magic_enum::enum_name(door_state));
    ImGui::Text("DoorState: %s", door_state_name.c_str());

    // if (open_hovered_doors) {
    //   if (door_state == SpaceshipDoorState::OPEN)
    //     door_state = SpaceshipDoorState::CLOSING;
    //   else if (door_state == SpaceshipDoorState::CLOSED)
    //     door_state = SpaceshipDoorState::OPENING;
    // }

    // Process state until it becomes open or closed
    const auto& closed_size = door.closed_size;
    const bool is_horizontal = closed_size.x > closed_size.y;
    const int closing_speed = 100;
    const float amount = closing_speed * dt;
    if (open) { // by opening, reduce size of door
      ImGui::Text("Opening...!");
      door.state = SpaceshipDoorState::BETWIXT;

      if (is_horizontal)
        door.to_close_increment.x -= amount;
      else
        door.to_close_increment.y -= amount;
    }
    if (close) { // by closing, increase size of door
      ImGui::Text("Closing...!");
      door.state = SpaceshipDoorState::BETWIXT;

      if (is_horizontal)
        door.to_close_increment.x += amount;
      else
        door.to_close_increment.y += amount;
    }

    // Update the door size
    {
      const auto store_float_overflow = [](float& amount) -> int {
        const int amount_int = glm::round(amount);
        amount -= amount_int;
        return amount_int;
      };
      const int x_change = store_float_overflow(door.to_close_increment.x);
      const int y_change = store_float_overflow(door.to_close_increment.y);

      // Set the initial width of the door
      if (glm::abs(x_change) > 0 && close && is_horizontal)
        door_aabb.size.y = closed_size.y;
      else if (glm::abs(y_change) > 0 && close && !is_horizontal)
        door_aabb.size.x = closed_size.x;

      set_size(r, e, { door_aabb.size.x + x_change, door_aabb.size.y + y_change });
    }

    // Check if door is closed
    //
    if (door_aabb.size.x <= 0.0f || door_aabb.size.y <= 0.0f) {
      door.state = SpaceshipDoorState::OPEN;
      set_size(r, e, { 0, 0 });
    }

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