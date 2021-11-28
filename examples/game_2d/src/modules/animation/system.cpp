// your header
#include "modules/animation/system.hpp"

// components
#include "modules/animation/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

// c++ lib
#include <math.h>

// Temporary
namespace game2d {

float
easeInOutQuad(float x)
{
  return x < 0.5f ? 2.0f * x * x : 1.0f - pow(-2.0f * x + 2.0f, 2.0f) / 2.0f;
};

} // namespace game2d

void
game2d::update_animation_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& view = registry.view<AnimationBounce, RenderSizeComponent>();
  view.each([&dt](auto& anim, auto& size) {
    anim.time += dt;

    static float animation_rate = 10.0f;
    static float animation_scale = 10.0f;

    // ImGui::Begin("Animation Editor");
    // if (ImGui::DragFloat("Rate", &animation_rate)) {
    //   // reset
    //   size.w = 16.0f;
    //   size.h = 16.0f;
    // }
    // if (ImGui::DragFloat("Scale", &animation_scale)) {
    //   // reset
    //   size.w = 16.0f;
    //   size.h = 16.0f;
    // }
    // ImGui::End();

    float amount = animation_scale * sin(animation_rate * anim.time) * dt;
    size.dh += amount;
    // size.dw += amount;

    int increase_h = static_cast<int>(size.dh);
    if (increase_h != 0) {
      size.h += increase_h;
      size.dh -= increase_h;
    }

    int increase_w = static_cast<int>(size.dw);
    if (increase_w != 0) {
      size.w += increase_w;
      size.dw -= increase_w;
    }
  });
};
