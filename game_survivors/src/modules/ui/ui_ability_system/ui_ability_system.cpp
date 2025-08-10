#include "pch.hpp"

#include "ui_ability_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/systems/system_ability/ability_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

void
update_ui_ability_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  ImGui::Begin("overlay", NULL, flags);

  const auto tl = ImGui::GetWindowPos();
  const auto wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();

  for (const auto& [e, ability_c, colour_c, t_c] :
       r.view<const AbilityComponent, const DefaultColour, const TransformComponent>().each()) {
    const bool allowed_to_use_ability_1 = ability_c.ability_1_cooldown_left <= 0.0f;
    const bool allowed_to_use_ability_2 = ability_c.ability_2_cooldown_left <= 0.0f;

    const auto my_cooldown_col = engine::SRGBColour(0.2f, 0.2f, 0.2f, 0.25f);
    const auto my_active_col = engine::SRGBColour(0.2f, 1.0f, 0.2f, 0.3f);
    const auto im_cooldown_col = convert_my_to_im(my_cooldown_col);
    const auto im_active_col = convert_my_to_im(my_active_col);

    const auto half_button_size = ImVec2{ 10, 10 };
    const auto button_size = ImVec2{ 20, 20 };
    const float space_between_buttons = 3.0f;
    const float icon_padding_x = 6.0f;
    const float icon_padding_y = 10.0f;
    const auto ability_1_spot = glm::vec2{ -t_c.scale.x - icon_padding_x, -t_c.scale.y - icon_padding_y };
    const auto ability_2_spot =
      glm::vec2{ -t_c.scale.x - icon_padding_x + +space_between_buttons + button_size.x, -t_c.scale.y - icon_padding_y };

    const auto pos1 = glm::vec2{ t_c.position.x + ability_1_spot.x, t_c.position.y + ability_1_spot.y };
    const auto pos2 = glm::vec2{ t_c.position.x + ability_2_spot.x, t_c.position.y + ability_2_spot.y };
    const auto screenspace1 = worldspace_to_screenspace(r, pos1);
    const auto screenspace2 = worldspace_to_screenspace(r, pos2);
    const auto im_screenspace1 = ImVec2(screenspace1.x, screenspace1.y);
    const auto im_screenspace2 = ImVec2(screenspace2.x, screenspace2.y);

    const auto button1_tl = ImVec2{ im_screenspace1.x - half_button_size.x, im_screenspace1.y - half_button_size.y };
    const auto button1_br = ImVec2{ im_screenspace1.x + half_button_size.x, im_screenspace1.y + half_button_size.y };
    const auto button1_col = allowed_to_use_ability_1 ? im_active_col : im_cooldown_col;
    draw_list->AddRectFilled(button1_tl, button1_br, button1_col, 8.0f);

    const auto button2_tl = ImVec2{ im_screenspace2.x - half_button_size.x, im_screenspace2.y - half_button_size.y };
    const auto button2_br = ImVec2{ im_screenspace2.x + half_button_size.x, im_screenspace2.y + half_button_size.y };
    const auto button2_col = allowed_to_use_ability_2 ? im_active_col : im_cooldown_col;
    draw_list->AddRectFilled(button2_tl, button2_br, button2_col, 8.0f);

    auto* font = get_inter_font(r, FontSize::TEXT_SMALL);
    const auto text_size1 = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, "LB");
    const auto text_size2 = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, "RB");
    const auto text_pos1 = ImVec2(im_screenspace1.x - 0.5f * text_size1.x, im_screenspace1.y - 0.5f * text_size1.y);
    const auto text_pos2 = ImVec2(im_screenspace2.x - 0.5f * text_size2.x, im_screenspace2.y - 0.5f * text_size2.y);
    draw_list->AddText(font, font->FontSize, text_pos1, IM_COL32(255, 255, 255, 255), "LB");
    draw_list->AddText(font, font->FontSize, text_pos2, IM_COL32(255, 255, 255, 255), "RB");

    // Sprite s1;
    // s1.sprite = allowed_to_use_ability_1 ? "CIRCLE" : "CIRCLE";
    // s1.col = allowed_to_use_ability_1 ? active_col : cooldown_col;
    // s1.pos = get_position(r, e) + ability_1_spot;
    // s1.size = { 8, 8 };
    // s1.z_idx = ZLayer::VFX;
    // draw_sprite(r, s1);

    // Sprite s2;
    // s2.sprite = allowed_to_use_ability_2 ? "CIRCLE" : "CIRCLE";
    // s2.col = allowed_to_use_ability_2 ? active_col : cooldown_col;
    // s2.pos = get_position(r, e) + ability_2_spot;
    // s2.size = { 8, 8 };
    // s2.z_idx = ZLayer::VFX;
    // draw_sprite(r, s2);

    // draw an anchor sprite
    if (ability_c.ability_1_in_progress) {
      const auto offset = glm::vec2{ -10, 10 };
      Sprite s;
      s.sprite = "PICKAXE"; // kinda looks like anchor if you rotate it
      // point pickaxe down and left
      constexpr float rotation = engine::PI + (30 * engine::Deg2Rad);
      // imgui_draw_float("rotation", rotation);
      s.z_rotation = rotation;
      s.col = colour_c.colour;
      s.pos = get_position(r, e) + offset;
      s.size = { 20, 20 };
      s.z_idx = ZLayer::VFX;
      draw_sprite(r, s);
    }
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d