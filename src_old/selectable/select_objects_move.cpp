// your header
#include "select_objects_move.hpp"

// components
#include "game/components/pathfinding.hpp"
#include "game/components/selectable.hpp"
#include "modules/camera/components.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "resources/colour.hpp"

// engine headers
#include "engine/colour.hpp"
#include "engine/maths/maths.hpp"

void
game2d::update_select_objects_move_system(entt::registry& registry)
{
  const auto& colours = registry.ctx().at<SINGLETON_ColoursComponent>();
  const auto& input = registry.ctx().at<SINGLETON_InputComponent>();
  const auto& mouse_pos = input.mouse_position_in_worldspace;

  //
  // Calculate average position of selected objects
  //
  int count = 0;
  glm::ivec2 avg_pos{ 0, 0 };
  {
    const auto& selectable = registry.view<const SelectableComponent, const TransformComponent>();
    selectable.each([&avg_pos, &count](const auto& s, const auto& transform) {
      if (!s.is_selected)
        return;
      count += 1;
      avg_pos = { avg_pos.x + transform.position.x, avg_pos.y + transform.position.y };
    });
    if (count != 0)
      avg_pos /= count;
  }

  //
  // Process Input...
  // Move all the selected units, and try and keep them in formation
  //
  entt::entity random_chosen_entity_parent = entt::null;

  if (get_mouse_rmb_press()) {

    const auto& selectable =
      registry.view<const SelectableComponent, TransformComponent, VelocityComponent, DestinationComponent>();
    selectable.each([&colours, &avg_pos, &mouse_pos, &random_chosen_entity_parent](
                      const auto entity, const auto& s, auto& transform, auto& vel, auto& destinationC) {
      if (!s.is_selected)
        return;
      random_chosen_entity_parent = entity; // doesn't matter which one

      // calculate current offset from avg position
      glm::ivec2 offset = { transform.position.x - avg_pos.x, transform.position.y - avg_pos.y };
      glm::ivec2 destination = mouse_pos + offset;

      transform.position = { mouse_pos.x, mouse_pos.y, 0.0f };

      // // set velocity to get to destination
      // glm::ivec2 d = { destination.x - transform.position.x, destination.y - transform.position.y };
      // if (d.x == 0 && d.y == 0) // check same spot not clicked
      //   return;
      // glm::vec2 dir = glm::vec2(d.x, d.y);
      // glm::vec2 n = normalize(dir);
      // const float speed = 200.0f;
      // vel.x = n.x * speed;
      // vel.y = n.y * speed;

      // set destination
      destinationC.xy = destination;
    });

    //
    // Draw a line from the chosen entity to the avg movement point
    //

    if (random_chosen_entity_parent != entt::null) {

      auto& destinationC = registry.get<DestinationComponent>(random_chosen_entity_parent);

      glm::ivec2 d = { mouse_pos.x - avg_pos.x, mouse_pos.y - avg_pos.y };
      if (d.x == 0 && d.y == 0) // check same spot not clicked
        return;
      glm::vec2 dir = glm::vec2(d.x, d.y);
      glm::vec2 n = normalize(dir);

      // convert dir to angle
      const float radius = glm::length(dir);
      const float angle = engine::dir_to_angle_radians(dir);
      const float x = avg_pos.x + (n.x * radius / 2.0f);
      const float y = avg_pos.y + (n.y * radius / 2.0f);

      // delete destination line if it already existed
      if (destinationC.destination_line != entt::null) {
        registry.destroy(destinationC.destination_line);
        destinationC.destination_line = entt::null;
      }

      // create line from current position to end position
      entt::entity e = registry.create();
      registry.emplace<TagComponent>(e, "SPAWNED LINE");
      TransformComponent transform;
      transform.position.x = x;
      transform.position.y = y;
      transform.scale.x = radius;
      transform.scale.y = 1;
      transform.rotation.z = angle;
      registry.emplace<TransformComponent>(e, transform);
      SpriteComponent sprite;
      sprite.colour = engine::SRGBToLinear(colours.red);
      sprite.x = 0;
      sprite.y = 0;
      registry.emplace<SpriteComponent>(e, sprite);

      destinationC.destination_line = e;
    }
  }
};
