// header
#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/helpers.hpp"

void
game2d::update_lifecycle_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>();
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);
    lifecycle.milliseconds_alive += static_cast<int>(milliseconds_dt);
  });

  // process destroyed objects
  for (const auto& entity : dead.dead)
    r.destroy(entity);

  dead.dead.clear();

  //
  // process create requests
  //
  dead.created_this_frame.clear();

  const auto requests = r.view<CreateEntityRequest>();
  for (auto [entity, request] : requests.each()) {
    auto e = create_gameplay(r, request.type);

    if (auto* vel = r.try_get<VelocityComponent>(e)) {
      vel->x = request.velocity.x;
      vel->y = request.velocity.y;
    }

    auto* sc = r.try_get<SpriteComponent>(e);
    if (sc && request.sprite.has_value()) {
      const auto& anims = get_first_component<SINGLETON_Animations>(r);
      const auto anim = find_animation(anims.animations, request.sprite.value());
      sc->x = anim.animation_frames[0].x;
      sc->y = anim.animation_frames[0].y;
    }

    // set position by transform
    auto& transform = r.get<TransformComponent>(e);
    transform.position = request.position;

    // capture new entity
    dead.created_this_frame.push_back(e);

    // destroy request
    r.destroy(entity);
  }
};