#include "scene_brawl_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_brawler/actor_brawler_components.hpp"
#include "modules/actor_brawler_spawner/actor_brawler_spawner_components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/effects/outline.hpp"
#include "modules/system_parallax_mouse/parallax_mouse_components.hpp"

#include <SDL_scancode.h>

namespace game2d {

void
create_walls(entt::registry& r)
{
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
};

entt::entity
create_brawler(entt::registry& r, const BrawlerSpawner& spawner, const glm::vec2 pos)
{
  const auto size = glm::ivec2(32, 32);

  auto e = r.create();
  r.emplace<TagComponent>(e, "brawler");
  r.emplace<SpriteComponent>(e);
  r.emplace<TransformComponent>(e);
  r.emplace<DefaultColour>(e, spawner.colour);

  // Add items to physics system?
  PhysicsDescription pdesc;
  pdesc.type = b2_dynamicBody;
  pdesc.size = size;
  pdesc.linear_damping = 10.0f;
  pdesc.angular_damping = 0.0f;
  pdesc.position = pos;
  // on fixture...
  pdesc.is_sensor = false;
  pdesc.density = 1.0f;
  pdesc.friction = 0.3f;
  // pdesc.restitution = 0.0f; // low to encourage intersect
  create_physics_actor(r, e, pdesc);

  set_size(r, e, size);
  set_sprite(r, e, "PERSON_30_6");
  // set_sprite(r, e, "EMPTY");
  set_position(r, e, pos);
  set_colour(r, e, spawner.colour);

  r.emplace<ActionKey>(e, spawner.key);
  r.emplace<SeparateTransformAndAABB>(e);
  // r.emplace<WiggleUpAndDown>(e);
  r.emplace<HealthComponent>(e, 5, 5);
  r.emplace<TeamIndexComponent>(e, spawner.team_idx);
  r.emplace<Brawler>(e);

  if (spawner.spawns_ai) {
    BrawlerAI ai;
    ai.time_between_punches_min = 0.3f;
    ai.time_between_punches_max = 0.3f;
    r.emplace<BrawlerAI>(e, ai);
  }

  r.emplace<SpriteOutline>(e);
  r.emplace<ParallaxMouseComponent>(e);

  return e;
}

} // namespace game2d