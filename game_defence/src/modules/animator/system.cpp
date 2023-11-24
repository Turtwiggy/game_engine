#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "glm/glm.hpp"

namespace game2d {

int
get_index(const float time, const float duration, const int size)
{
  if (duration == 0.0f)
    return 0;
  const float r = time / duration;    // a value between 0 and 1
  const int s = glm::floor(r * size); // a value between 0 and size
  return glm::clamp(s, 0, size);      // check between 0 and size
};

void
update_animator_system(entt::registry& r, float dt)
{
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& anim = find_animation(anims, "MUZZLE_FLASH");
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<SpriteComponent, SpriteAnimationComponent>();
  for (const auto& [e, sprite, animation] : view.each()) {

    // loop timer between 0 and duration
    animation.timer += dt;

    // should end or loop the animation?
    if (animation.timer >= animation.duration && !animation.looping) {
      dead.dead.emplace(e); // destroy this effect
      continue;
    }

    // loop the timer
    animation.timer = fmod(animation.timer, animation.duration);

    // get the index of the frame to play
    const int i0 = get_index(animation.timer, animation.duration, anim.animation_frames.size());
    // const int i1 = i0 + 1;

    const SpritePosition& frame = anim.animation_frames[i0];
    sprite.tex_pos = frame;
  }
}

} // namespace game2d