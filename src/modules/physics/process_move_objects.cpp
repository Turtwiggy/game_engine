// your header
#include "modules/physics/process_move_objects.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"

// c++ lib headers
#include <algorithm>
#include <vector>

void
game2d::update_move_objects_system(entt::registry& registry, uint64_t milliseconds_dt)
{
  // actors and solids never overlap,
  // and solids dont overlap with solids

  std::vector<PhysicsObject> solids;
  get_solids_as_physics_objects(registry, solids);

  // move actors, but stop at solids
  const auto& actors =
    registry
      .view<TransformComponent, const VelocityComponent, const PhysicsSizeComponent, const PhysicsActorComponent>();

  float seconds_dt = milliseconds_dt / 1000.0f;
  actors.each([&solids, &seconds_dt](TransformComponent& transform,
                                     const VelocityComponent& vel,
                                     const PhysicsSizeComponent& size,
                                     const PhysicsActorComponent& actor) {
    transform.position_dxdy.x += vel.x * seconds_dt;
    transform.position_dxdy.y += vel.y * seconds_dt;

    int move_x = static_cast<int>(transform.position_dxdy.x);
    do_move_x(transform, solids, move_x);

    int move_y = static_cast<int>(transform.position_dxdy.y);
    do_move_y(transform, solids, move_y);
  });

  // move grid actors, but stop at solids
  const auto& grid_actors =
    registry.view<TransformComponent, GridMoveComponent, const PhysicsSizeComponent, const PhysicsActorComponent>();
  grid_actors.each([&solids](TransformComponent& transform,
                             GridMoveComponent& grid,
                             const PhysicsSizeComponent& size,
                             const PhysicsActorComponent& actor) {
    transform.position_dxdy.x += grid.x;
    transform.position_dxdy.y += grid.y;
    grid.x = 0;
    grid.y = 0;

    int move_x = static_cast<int>(transform.position_dxdy.x);
    do_move_x(transform, solids, move_x);

    int move_y = static_cast<int>(transform.position_dxdy.y);
    do_move_y(transform, solids, move_y);
  });

  // move solids
  // A solid interacts with an actor by carrying or pushing
  // An actor is carried if it is riding a solid
  // An actor is pushed if the solid's movement results in them overlapping
  // Pushing takes priority over carrying
};