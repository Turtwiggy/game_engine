#include "pch.hpp"

#include "animator_system.hpp"

#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"

namespace game2d {

int
get_index(const float time, const float duration, const int size)
{
  if (duration == 0.0f)
    return 0;
  const float r = time / duration;                      // a value between 0 and 1
  const int s = static_cast<int>(glm::floor(r * size)); // a value between 0 and size
  return glm::clamp(s, 0, size);                        // check between 0 and size
};

void
update_animator_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& anims = SINGLE_Animations::instance;

  // #if defined(_DEBUG)
  //   static float duration = 1.0f;
  //   bool duration_changed = false;
  //   if (imgui_draw_float("duration", duration)) {
  //     SDL_Log("duration changeed...");
  //     duration_changed = true;
  //   }
  // #endif

  const auto& view = r.view<SpriteComponent, SpriteAnimationState>();
  for (const auto& [e, sprite_c, animation] : view.each()) {
    if (animation.cached_playing_animation != animation.playing_animation_name) {
      auto [a, b] = find_animation(anims, animation.playing_animation_name);
      animation.cached_idx_a = a; // a is the index of the sprite texture
      animation.cached_idx_b = b; // b is the index of the sprite animation in the spritesheet
      animation.cached_playing_animation = animation.playing_animation_name;
    }
    const auto& anim = anims.animations[animation.cached_idx_a].second[animation.cached_idx_b];

    // #if defined(_DEBUG)
    //     // debug: change animation speed;
    //     if (duration_changed)
    //       animation.duration = duration;
    // #endif

    // loop timer between 0 and duration
    animation.timer += dt;
    // ImGui::Text("animation timer: %f", animation.timer);

    const int n_frames = (int)anim.animation_frames.size();

    // pause on final frame if not looping
    if (animation.timer >= animation.duration && !animation.looping) {
      const int i0 = (int)(n_frames - 1);
      const SpritePosition& frame = anim.animation_frames[i0];
      sprite_c.tex_pos = frame;
      continue;
    }

    // loop the timer
    if (animation.timer >= animation.duration)
      animation.timer -= animation.duration * glm::floor(animation.timer / animation.duration);

    // get the index of the frame to play
    const int i0 = get_index(animation.timer, animation.duration, n_frames);

    // ImGui::Text("i0: %i x: %i y: %i, w: %i h: %i", i0, hmm.x, hmm.y, hmm.w, hmm.h);
    sprite_c.tex_pos = anim.animation_frames[i0];
  }
}

} // namespace game2d