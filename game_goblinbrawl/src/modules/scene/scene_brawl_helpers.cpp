#include "scene_brawl_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/colour/components.hpp"

namespace game2d {

void
create_walls(entt::registry& r)
{
  // Create 4 walls for the cage
  auto create_wall = [&r](glm::vec2 pos, glm::vec2 size) {
    auto e = r.create();
    r.emplace<TagComponent>(e, "wall");
    r.emplace<SpriteComponent>(e);
    r.emplace<TransformComponent>(e);
    r.emplace<DefaultColour>(e);
    set_size(r, e, size);
    set_sprite(r, e, "EMPTY");
    set_position(r, e, pos);
    set_colour(r, e, { 0.0f, 0.0f, 0.0f, 0.0f });

    // Add items to physics system
    PhysicsDescription pdesc;
    pdesc.type = b2_staticBody;
    pdesc.size = size;
    pdesc.position = pos;
    create_physics_actor(r, e, pdesc);
  };
  create_wall({ 0, 100 }, { 300, 10 });
  create_wall({ 0, -100 }, { 300, 10 });
  create_wall({ 160, 0 }, { 10, 400 });
  create_wall({ -160, 0 }, { 10, 400 });
}

} // namespace game2d