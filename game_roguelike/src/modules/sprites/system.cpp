// header
#include "system.hpp"
#include "helpers.hpp"

#include "maths/maths.hpp"

namespace game2d {

void
update_sprite_system(const GameEditor& editor, Game& game, float dt)
{
  // const auto& anims = editor.animations;
  auto& registry = game.state;

  // { // set sprite animation by velocity
  //   const auto& view =
  //     registry.view<SpriteAnimationComponent, SpriteComponent, AnimationSetByVelocityComponent, VelocityComponent>();
  //   view.each([&anims, &registry](auto& anim, auto& sprite, const auto& asbv, const auto& vel) {
  //     std::string animation = "down_idle";

  //     float angle = engine::dir_to_angle_radians({ vel.x, vel.y });
  //     if (angle < 0.25f * engine::PI || angle > 1.75f * engine::PI)
  //       animation = "left_walk_cycle";
  //     else if (angle < 0.75f * engine::PI)
  //       animation = "up_walk_cycle";
  //     else if (angle < 1.25f * engine::PI)
  //       animation = "right_walk_cycle";
  //     else
  //       animation = "down_walk_cycle";

  //     if (animation != anim.playing_animation_name) {
  //       // immediately play new anim
  //       anim.frame = 0;
  //       anim.frame_dt = 0.0f;
  //       anim.playing_animation_name = animation;

  //       // set starting frame
  //       const auto& anim_data = find_animation(anims.animations, anim.playing_animation_name);
  //       sprite.x = anim_data.animation_frames[0].x;
  //       sprite.y = anim_data.animation_frames[0].y;
  //     }
  //   });
  // }

  // { // update sprite component with sprite animation
  //   const auto& view = registry.view<SpriteComponent, SpriteAnimationComponent>();
  //   view.each([&anims, &dt](SpriteComponent& sprite, SpriteAnimationComponent& animation) {
  //     //
  //     SpriteAnimation current_animation = find_animation(anims.animations, animation.playing_animation_name);

  //     if (!animation.playing)
  //       return;

  //     int frames = current_animation.animation_frames.size();

  //     animation.frame_dt += dt * animation.speed;
  //     if (animation.frame_dt >= current_animation.animation_frames_per_second) {
  //       // next frame!
  //       animation.frame_dt -= current_animation.animation_frames_per_second;
  //       animation.frame += 1;

  //       if (animation.frame >= frames && !animation.looping)
  //         animation.playing = false;

  //       animation.frame %= frames;
  //       sprite.x = current_animation.animation_frames[animation.frame].x;
  //       sprite.y = current_animation.animation_frames[animation.frame].y;
  //       sprite.angle_radians = glm::radians(current_animation.animation_angle_degrees);
  //     }
  //   });
  // }
};

} // namespace game2d