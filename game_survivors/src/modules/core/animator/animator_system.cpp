#include "animator_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"

namespace game2d {

int
get_index(const float time, const float duration, const int size)
{
  if (duration == 0.0f)
    return 0;
  const float r = time / duration;                      // a value between 0 and 1
  const int s = static_cast<int>(std::floor(r * size)); // a value between 0 and size
  return std::clamp(s, 0, size);                        // check between 0 and size
};

void
update_animator_system(entt::registry& r, const float dt)
{
  const auto& anims = get_first_component<SINGLE_Animations>(r);

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
    const auto& [spritesheet, anim] = find_animation(anims, animation.playing_animation_name);

    // #if defined(_DEBUG)
    //     // debug: change animation speed;
    //     if (duration_changed)
    //       animation.duration = duration;
    // #endif

    // loop timer between 0 and duration
    animation.timer += dt;
    // ImGui::Text("animation timer: %f", animation.timer);

    // should end or loop the animation?
    if (animation.timer >= animation.duration && !animation.looping) {
      // dead.dead.push_back(e); // destroy this effect
      continue;
    }

    // pause on final frame if not looping
    if (animation.timer >= animation.duration && !animation.looping) {
      const int i0 = static_cast<int>(anim.animation_frames.size() - 1);
      const SpritePosition& frame = anim.animation_frames[i0];
      sprite_c.tex_pos = frame;
      continue;
    }

    // loop the timer
    animation.timer = fmod(animation.timer, animation.duration);

    // get the index of the frame to play
    const int i0 = get_index(animation.timer, animation.duration, static_cast<int>(anim.animation_frames.size()));

    const auto& hmm = anim.animation_frames[i0];
    // ImGui::Text("i0: %i x: %i y: %i, w: %i h: %i", i0, hmm.x, hmm.y, hmm.w, hmm.h);
    sprite_c.tex_pos = hmm;
  }
}

} // namespace game2d