// header
#include "system.hpp"

#include "engine/maths/maths.hpp"
#include "engine/opengl/texture.hpp"
#include "modules/physics/components.hpp"
#include "modules/sprites/helpers.hpp"
#include "resources/textures.hpp"

namespace game2d {

void
init_sprite_system(entt::registry& registry)
{
  auto& anim = registry.ctx().emplace<SINGLETON_Animations>();
  auto& tex = registry.ctx().at<SINGLETON_Textures>();

  // load textures
  std::vector<std::pair<int, std::string>> textures_to_load;

  // iterate over user config
  for (int i = 0; i < tex.textures.size(); i++) {
    auto& t = tex.textures[i];
    if (t.path != "") {
      textures_to_load.emplace_back(t.tex_unit, t.path);
    }
  }

  auto tex_ids = engine::load_textures_threaded(textures_to_load);

  for (int i = 0; i < tex.textures.size(); i++) {
    auto& t = tex.textures[i];

    // set texture ids
    if (t.path != "")
      t.tex_id = tex_ids[i];

    // load sprite info from texture if it exists
    if (t.spritesheet_path != "")
      load_sprites(anim.animations, t.spritesheet_path);
  }
}

void
update_sprite_system(entt::registry& registry, float dt)
{
  const auto& anims = registry.ctx().at<SINGLETON_Animations>();

  { // set sprite animation by velocity
    const auto& view =
      registry.view<SpriteAnimationComponent, SpriteComponent, AnimationSetByVelocityComponent, VelocityComponent>();
    view.each([&anims, &registry](auto& anim, auto& sprite, const auto& asbv, const auto& vel) {
      std::string animation = "down_idle";

      float angle = engine::dir_to_angle_radians({ vel.x, vel.y });
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

  { // update sprite component with sprite animation
    const auto& view = registry.view<SpriteComponent, SpriteAnimationComponent>();
    view.each([&anims, &dt](SpriteComponent& sprite, SpriteAnimationComponent& animation) {
      //
      SpriteAnimation current_animation = find_animation(anims.animations, animation.playing_animation_name);

      if (!animation.playing)
        return;

      int frames = current_animation.animation_frames.size();

      animation.frame_dt += dt * animation.speed;
      if (animation.frame_dt >= current_animation.animation_frames_per_second) {
        // next frame!
        animation.frame_dt -= current_animation.animation_frames_per_second;
        animation.frame += 1;

        if (animation.frame >= frames && !animation.looping)
          animation.playing = false;

        animation.frame %= frames;
        sprite.x = current_animation.animation_frames[animation.frame].x;
        sprite.y = current_animation.animation_frames[animation.frame].y;
        sprite.angle_radians = current_animation.animation_angle_degrees * engine::PI / 180.0f;
      }
    });
  }
};

} // namespace game2d