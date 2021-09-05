// your header
#include "screen_shake_system.hpp"

#include "components/screen_shake.hpp"

namespace game2d {

void
update_screen_shake_system(entt::registry& registry, float dt){
  // auto& resources = registry.ctx<Resources>();
  // auto& screenshake = registry.ctx<ScreenShakeTimer>();

  // if (screenshake.time_left > 0.0f) {
  //   screenshake.time_left -= dt;
  //   resources.instanced_quad_shader.bind();
  //   resources.instanced_quad_shader.set_bool("shake", true);
  // }
  // if (screenshake.time_left <= 0.0f) {
  //   resources.instanced_quad_shader.bind();
  //   resources.instanced_quad_shader.set_bool("shake", false);
  // }
};

} // namespace game2d