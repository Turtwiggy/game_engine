// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// c++ lib headers
#include <algorithm>
#include <vector>

void
game2d::update_move_objects_system(entt::registry& r, uint64_t milliseconds_dt)
{
  // update the physics view of the world
  auto physics_objects_view = r.view<PhysicsTransformComponent, const TransformComponent>();
  physics_objects_view.each([](auto entity, PhysicsTransformComponent& ptc, const TransformComponent& tc) {
    ptc.x_tl = static_cast<int>(tc.position.x - (ptc.w / 2.0f));
    ptc.y_tl = static_cast<int>(tc.position.y - (ptc.h / 2.0f));
    ptc.ent_id = static_cast<uint32_t>(entity);
  });

  // move grid actors,
  // stop if collides with an entity with the blocking component

  std::vector<Collision2D> would_collide_x;
  std::vector<Collision2D> would_collide_y;

  const auto& grid_actors = r.view<TransformComponent, GridMoveComponent, const PhysicsTransformComponent>();
  grid_actors.each(
    [&r, &would_collide_x, &would_collide_y](entt::entity entity, auto& transform, auto& grid, const auto& ptc) {
      transform.position_dxdy.x += grid.x;
      transform.position_dxdy.y += grid.y;
      grid.x = 0;
      grid.y = 0;

      int move_x = static_cast<int>(transform.position_dxdy.x);
      do_move(r, entity, move_x, transform, ptc, CollisionAxis::x, would_collide_x);

      int move_y = static_cast<int>(transform.position_dxdy.y);
      do_move(r, entity, move_y, transform, ptc, CollisionAxis::y, would_collide_y);
    });

  if (would_collide_x.size() != 0)
    printf("would be %i collisions on x axis", would_collide_x.size());
  if (would_collide_y.size() != 0)
    printf("would be %i collisions on y axis", would_collide_y.size());

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
};