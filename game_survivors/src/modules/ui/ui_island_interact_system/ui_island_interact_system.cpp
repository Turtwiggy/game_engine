#include "pch.hpp"

#include "ui_island_interact_components.hpp"
#include "ui_island_interact_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_ui_island_interact_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  return;

  static float time_to_complete = 2.0f;
  static float progress = 0.0f;
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const bool yes_held = std::find(b_s.begin(), b_s.end(), ActionStateEnum::HELD) != b_s.end();
  const bool not_held = std::find(b_s.begin(), b_s.end(), ActionStateEnum::HELD) == b_s.end();
  if (yes_held)
    progress += dt;
  if (not_held)
    progress -= 2.0f * dt; // release is 2x as fast as held
  progress = glm::clamp(progress, 0.0f, time_to_complete);

  const float progress_0_1 = (progress / time_to_complete);
  if ((progress - time_to_complete) > 0.0f)
    progress = 0.0f;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  const auto& ri = SINGLE_RendererInfo::instance;
  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  imgui_begin("overlay", ImGuiWindowFlags_NoInputs);
  const auto tl = ImGui::GetWindowPos();
  const auto wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();

  // For the moment, just have one button in the middle/center of the screen.
  auto font_size = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);

  const auto view = r.view<TransformComponent, InteractableComponent>();
  for (const auto& [e, t_c, i_c] : view.each()) {
    auto ws_pos = glm::vec2{ t_c.position.x, t_c.position.y };

    // static float timer = 0.0f;
    // timer += dt;
    // ws_pos.y += sin(timer);

    // offset y so you can see the thing you're interacting with
    ws_pos.y -= 1.5f * t_c.scale.y;

    // const auto ws_pos = glm::vec2{ 0, 0 };
    const auto ss_pos = worldspace_to_screenspace(r, ws_pos);
    const auto im_ss_pos = ImVec2(ss_pos.x, ss_pos.y);
    const auto text = std::string{ "HOLD" };
    const auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, text.c_str());

    const auto text_pos = center_text(font, font_size, text, im_ss_pos);
    const auto text_wh = calc_wh(text_pos, text_pos + text_size);
    const auto p_tl = text_pos;
    const auto p_br = text_pos + text_wh;
    const auto pad_tl = ImVec2{ p_tl.x - 20.0f, p_tl.y - 3 };
    const auto pad_br = ImVec2{ p_br.x + 20.0f, p_br.y + 3 };
    const auto inner_pad_tl = ImVec2{ p_tl.x - 20.0f + 4, p_tl.y - 3 + 3 };
    const auto inner_pad_br = ImVec2{ p_br.x + 20.0f - 4, p_br.y + 3 - 3 };
    const auto inner_pad_wh = inner_pad_br - inner_pad_tl;

    // use an easing function.

    const auto easeOutCubic = [](float x) -> float { return 1 - pow(1 - x, 3); };
    const auto easeInOutCubic = [](float x) -> float { return x < 0.5f ? 4.0f * x * x * x : 1 - pow(-2 * x + 2, 3) / 2.0f; };
    const auto ease_func = easeOutCubic;
    const float percent = ease_func(progress_0_1);

    // background.
    float rounding = 0.0f;
    float thickness = 1.0f;
    draw_list->AddRectFilled(pad_tl, pad_br, IM_COL32(0, 0, 0, 100), rounding);
    draw_list->AddRect(pad_tl, pad_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

    // text
    draw_list->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), text.c_str());

    // inner-border
    const auto inner_pad_br_percent = inner_pad_tl + ImVec2{ inner_pad_wh.x * percent, inner_pad_wh.y };
    draw_list->AddRectFilled(inner_pad_tl, inner_pad_br_percent, IM_COL32(255, 255, 255, 255));

    if (progress_0_1 >= 1.0)
      i_c.action(); // do the interact thing
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d