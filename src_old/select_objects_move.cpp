// your header
#include "select_objects_move.hpp"

// components
#include "components/pathfinding.hpp"
#include "components/selectable.hpp"
#include "resources/colour/colour.hpp"
#include "camera/components.hpp"
#include "events/components.hpp"
#include "events/helpers/mouse.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

// engine headers
#include "colour/colour.hpp"
#include "maths/maths.hpp"

void
game2d::update_select_objects_move_system(GameEditor& editor, Game& game)
{
  const auto& colours = editor.colours;
  const auto& input = game.input;
  auto& registry = game.state;
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

  groups.each([&registry](const auto& group, auto& transform, const auto& physics_size) {
    int group_units = group.units.size();
    if (group_units == 0)
      return;
    float& group_angle = transform.rotation.z;

    // TEMP: slowly rotate
    // group_angle += engine::PI / 8.0f * dt;

    int half_units = group_units / 2;
    int unit_index = -half_units; // go from -half_units to half_units

    int larger_side = physics_size.w > physics_size.h ? physics_size.w : physics_size.h;

    int unit_offset = 0;
    if (half_units != 0)
      unit_offset = (larger_side / 2) / half_units;

    for (const auto& unit : group.units) {
      auto& unittransform = registry.get<TransformComponent>(unit);
      auto& unitpath = registry.get<DestinationComponent>(unit);
      auto& unitvel = registry.get<VelocityComponent>(unit);

      glm::ivec2 unitdir = engine::angle_radians_to_direction(group_angle);
      glm::ivec2 destination = transform.position - unittransform.position;
      destination += (unitdir * unit_index * unit_offset);
      unit_index += 1;

      // check same spot not clicked
      glm::ivec2 d = { destination.x, destination.y };
      if (d.x == 0 && d.y == 0)
        continue;

      // dont go back and forth on a pixel
      if (glm::abs(d.x) <= 1 && glm::abs(d.y) <= 1)
        continue;

      // set velocity to get to destination
      glm::vec2 dir = { d.x, d.y };
      glm::vec2 n = normalize(dir);
      const float speed = 100.0f;
      unitvel.x = n.x * speed;
      unitvel.y = n.y * speed;

      // set destination
      unitpath.xy = destination;
    }
  });
};
