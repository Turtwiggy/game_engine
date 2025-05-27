#include "pch.hpp"

#include "effects_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_alpha_based_on_lifecycle/alpha_based_on_lifecycle_components.hpp"

namespace game2d {

entt::entity
spawn_fx(entt::registry& r, std::string name, glm::vec2 pos, glm::vec2 size, float sprite_fps)
{
  const auto effect_e = spawn(r, "empty");
  give_life(r, effect_e, pos, size);
  r.remove<OnDeathCallbacks>(effect_e); // not sure if this is correct

  SpriteAnimationState anim_c;
  anim_c.playing_animation_name = name;
  set_sprite(r, effect_e, anim_c.playing_animation_name);

  const auto& anims = SINGLE_Animations::instance;
  const auto& [spritesheet, anim] = find_animation(anims, anim_c.playing_animation_name);
  anim_c.duration = (1.0f / sprite_fps) * anim.animation_frames.size();
  anim_c.looping = false;

  r.emplace<SpriteAnimationState>(effect_e, anim_c);
  EntityTimedLifecycle lifecycle_c{ .milliseconds_alive_max = (int)(anim_c.duration * 1000) };
  r.emplace<EntityTimedLifecycle>(effect_e, lifecycle_c);
  r.emplace<SetAlphaBasedOnLifecycleComponent>(effect_e);

  set_z_index(r, effect_e, ZLayer::VFX);
  return effect_e;
}

} // namespace game2d