#include "pch.hpp"

#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "ui_debug_animations_system.hpp"

namespace game2d {

std::tuple<entt::entity, SpriteAnimation>
spawn_sprite(entt::registry& r, std::string name, glm::vec2 pos, float sprite_fps, bool looping)
{
  const auto& anims = SINGLE_Animations::instance;
  const auto [a, b] = find_animation(anims, name);
  const auto spritesheet = anims.animations[a].first;
  const auto& anim = anims.animations[a].second[b];
  const auto& frames = anim.animation_frames;

  const auto size = glm::vec2{ 16 * frames[0].w, 16 * frames[0].h };

  const auto effect_e = spawn(r, "empty");
  give_life(r, effect_e, pos, size);
  r.remove<OnDeathCallbacks>(effect_e); // not sure if this is correct

  SpriteAnimationState anim_c;
  anim_c.playing_animation_name = name;
  set_sprite(r, effect_e, anim_c.playing_animation_name);

  anim_c.duration = (1.0f / sprite_fps) * anim.animation_frames.size();
  anim_c.looping = looping;
  r.emplace<SpriteAnimationState>(effect_e, anim_c);

  if (!anim_c.looping) {
    EntityTimedLifecycle lifecycle_c{ .milliseconds_alive_max = (int)(anim_c.duration * 1000) };
    r.emplace<EntityTimedLifecycle>(effect_e, lifecycle_c);
  }

  return { effect_e, anim };
}

void
update_ui_debug_animations_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  const auto effects_ui = gesert_menubar_state(menu_c, "DebugSprites");
  if (!effects_ui.enabled)
    return;

  imgui_begin("DebugAnimations");

  static int sprite_fps = 12;
  static bool looping = true;
  static bool loaded_animations = false;
  static std::vector<std::string> animations;
  static int idx;
  static glm::vec2 anim_pos; // stop spawning on top of eachother

  if (!loaded_animations) {
    const SINGLE_Animations& anims = SINGLE_Animations::instance;
    for (const auto& [spritesheet, anims] : anims.animations) {
      if (spritesheet.name != "animated")
        continue;
      for (const auto& anim : anims)
        animations.push_back(anim.name);
    }
    loaded_animations = true;
  }

  imgui_draw_int("sprite_fps", sprite_fps);
  imgui_draw_bool("looping", looping);

  WomboComboIn combo_in(animations);
  combo_in.label = "animations";
  combo_in.current_index = idx;
  idx = draw_wombo_combo(combo_in).selected;

  if (ImGui::Button("SpawnSprite")) {
    const std::string animation_to_play = animations[idx];
    auto [e, anim] = spawn_sprite(r, animation_to_play, anim_pos, sprite_fps, looping);
    anim_pos.x += anim.animation_frames[0].w * 16;
  }

  auto view = r.view<SpriteAnimation>();
  ImGui::Text("Animations: %i", (int)view.size());

  ImGui::Separator();

  ImGui::Text("spawnpos: %f %f", anim_pos.x, anim_pos.y);
  if (ImGui::Button("NewLine")) {
    anim_pos.x = 0;
    anim_pos.y += 16;
  }
  if (ImGui::Button("ClearSprites")) {
    // auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    // dead.dead.insert(dead.dead.end(), view.begin(), view.end());
    r.destroy(view.begin(), view.end());
  }

  ImGui::End();
}

} // namespace game2d