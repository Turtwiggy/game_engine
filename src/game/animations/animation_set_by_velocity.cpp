#include "animation_set_by_velocity.hpp"

#include "components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/helpers.hpp"

#include "engine/maths/maths.hpp"

namespace game2d {

void
update_animation_set_by_velocity_system(entt::registry& registry)
{
  const auto& anims = registry.ctx().at<SINGLETON_Animations>();

  const auto& view = registry.view<SpriteAnimationComponent,
                                   SpriteComponent,
                                   AnimationSetByVelocityComponent,
                                   TransformComponent,
                                   VelocityComponent>();

  view.each([&anims, &registry](auto& anim, auto& sprite, const auto& asbv, const auto& transform, const auto& vel) {
    std::string animation = "down_idle";

    float angle = engine::dir_to_angle_radians({ vel.x, vel.y });

    // debug line
    // entt::entity& line = sprite.debug_line;
    // auto& line_transform = registry.get<TransformComponent>(line);
    // line_transform.position = transform.position;
    // line_transform.rotation.z = angle;
    // line_transform.scale = { 10, 1, 0 };

    if (angle < 0.25f * engine::PI || angle > 1.75f * engine::PI)
      animation = "left_walk_cycle";
    else if (angle < 0.75f * engine::PI)
      animation = "up_walk_cycle";
    else if (angle < 1.25f * engine::PI)
      animation = "right_walk_cycle";
    else
      animation = "down_walk_cycle";

    if (animation != anim.playing_animation_name) {
      // immediately play new anim
      anim.frame = 0;
      anim.frame_dt = 0.0f;
      anim.playing_animation_name = animation;

      // set starting frame
      const auto& anim_data = find_animation(anims.animations, anim.playing_animation_name);
      sprite.x = anim_data.animation_frames[0].x;
      sprite.y = anim_data.animation_frames[0].y;
    }
  });
}

} // namespace game2d