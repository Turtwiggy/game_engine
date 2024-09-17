#include "system.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"

namespace game2d {

float
flip(float x)
{
  return 1.0f - x;
};

float
ease_in(float t)
{
  return t * t;
};

float
ease_out(float t)
{
  return flip(flip(t) * flip(t)); // not sure this is correct
};

float
ease_in_out(float t)
{
  return engine::lerp(ease_in(t), ease_out(t), t);
}

// spike is a lerp that returns to it's original position
float
spike(float t)
{
  if (t <= .5f)
    return ease_in(t / 0.5f);

  return ease_in(flip(t) / 0.5f);
};

void
enable_flash(entt::registry& r, entt::entity e)
{
  set_colour(r, e, get_srgb_colour_by_tag(r, "enemy_flash"));
};

void
disable_flash(entt::registry& r, entt::entity e)
{
  const auto& default_c = r.get<DefaultColour>(e);
  set_colour(r, e, default_c.colour);
};

// note: while the thing is scaling, also make it flash
void
update_combat_scale_on_hit_system(entt::registry& r, const float dt)
{
  const auto& view =
    r.view<TransformComponent, RequestHitScaleComponent, const PhysicsBodyComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, t_c, req_c, pb_c] : view.each()) {
    //
    if (req_c.t == 0.0f)
      enable_flash(r, e);

    // variables
    const float speed = 5.0f;
    const float scale_up = 20.0f;

    req_c.t += dt * speed;

    const float scale = spike(req_c.t);
    const float new_size = get_size(r, e).x + scale_up * scale;

    // assume: x and y are the same size
    t_c.scale = { new_size, new_size, 1.0f };

    if (req_c.t >= 1.0f) {

      disable_flash(r, e);

      // reset the transform
      auto physics_size = get_size(r, e);
      t_c.scale = { physics_size.x, physics_size.y, 1.0f };

      r.remove<RequestHitScaleComponent>(e);
    }
  }

  //
}

} // namespace game2d