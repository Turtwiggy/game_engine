#include "pch.hpp"

#include "ui_ability_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/systems/system_ability/ability_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_ui_ability_system(entt::registry& r, float dt)
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

  for (const auto& [e, ability_c, colour_c, t_c, player_c] :
       r.view<AbilityComponent, const DefaultColour, const TransformComponent, const PlayerComponent>().each()) {
    const bool allowed_to_use_ability_1 = ability_c.ability_1_cooldown_left <= 0.0f;
    const bool allowed_to_use_ability_2 = ability_c.ability_2_cooldown_left <= 0.0f;

    if (allowed_to_use_ability_1 && ability_c.ability_1_flash_when_off_cooldown) {
      ability_c.ability_1_flash_when_off_cooldown = false;
      ability_c.ability_1_alpha = 1.0f;
    }
    if (allowed_to_use_ability_2 && ability_c.ability_2_flash_when_off_cooldown) {
      ability_c.ability_2_flash_when_off_cooldown = false;
      ability_c.ability_2_alpha = 1.0f;
    }

    ability_c.ability_1_alpha -= 1.0f * dt;
    ability_c.ability_1_alpha = glm::clamp(ability_c.ability_1_alpha, 0.0f, 1.0f);
    if (!ability_c.ability_1_pressed)
      ability_c.ability_1_alpha = 1.0f;
    ability_c.ability_2_alpha -= 1.0f * dt;
    ability_c.ability_2_alpha = glm::clamp(ability_c.ability_2_alpha, 0.0f, 1.0f);
    if (!ability_c.ability_2_pressed)
      ability_c.ability_2_alpha = 1.0f;

    const auto my_cooldown_col = engine::SRGBColour(0.2f, 0.2f, 0.2f, 0.25f);
    const auto my_active_col = default_player_colours[player_c.colour_idx];
    const auto im_cooldown_col = convert_my_to_im(my_cooldown_col);
    const auto im_active_col = convert_my_to_im(my_active_col);

    // auto button_size = ImVec2{ 24, 24 };
    // auto half_button_size = ImVec2{ 12, 12 };
    // // dont show full button after initial controls
    // if (ability_c.ability_1_pressed && ability_c.ability_2_pressed) {
    //   button_size = { 12, 12 };
    //   half_button_size = { 6, 6 };
    // }
    auto button_size = ImVec2{ 16, 16 };
    auto half_button_size = ImVec2{ 8, 8 };

    const float space_between_buttons = 4.0f;
    const float icon_padding_x = 6.0f;
    const float icon_padding_y = 10.0f;
    const auto ability_1_spot = glm::vec2{ -t_c.scale.x - icon_padding_x, -t_c.scale.y - icon_padding_y };
    const auto ability_2_spot =
      glm::vec2{ -t_c.scale.x - icon_padding_x + space_between_buttons + button_size.x, -t_c.scale.y - icon_padding_y };

    const auto pos1 = glm::vec2{ t_c.position.x + ability_1_spot.x, t_c.position.y + ability_1_spot.y };
    const auto pos2 = glm::vec2{ t_c.position.x + ability_2_spot.x, t_c.position.y + ability_2_spot.y };
    const auto screenspace1 = worldspace_to_screenspace(r, pos1);
    const auto screenspace2 = worldspace_to_screenspace(r, pos2);
    const auto im_screenspace1 = ImVec2(screenspace1.x, screenspace1.y);
    const auto im_screenspace2 = ImVec2(screenspace2.x, screenspace2.y);
    const float rounding = 16.0f;

    const auto button1_tl = ImVec2{ im_screenspace1.x - half_button_size.x, im_screenspace1.y - half_button_size.y };
    const auto button1_br = ImVec2{ im_screenspace1.x + half_button_size.x, im_screenspace1.y + half_button_size.y };
    auto my_button1_col = my_active_col;
    my_button1_col.a = (int)(ability_c.ability_1_alpha * 255.0f);
    const auto im_button1_col = convert_my_to_im(my_button1_col);
    draw_list->AddRectFilled(button1_tl, button1_br, im_button1_col, rounding);
    // draw_list->AddRect(button1_tl, button1_br, im_window_border_col, rounding);

    const auto button2_tl = ImVec2{ im_screenspace2.x - half_button_size.x, im_screenspace2.y - half_button_size.y };
    const auto button2_br = ImVec2{ im_screenspace2.x + half_button_size.x, im_screenspace2.y + half_button_size.y };
    auto my_button2_col = my_active_col;
    my_button2_col.a = (int)(ability_c.ability_2_alpha * 255.0f);
    const auto im_button2_col = convert_my_to_im(my_button2_col);
    draw_list->AddRectFilled(button2_tl, button2_br, im_button2_col, rounding);
    // draw_list->AddRect(button2_tl, button2_br, im_window_border_col, rounding);

    auto lb_text = player_c.lb_cached;
    // if (lb_text == "")
    //   lb_text = "LMB";
    auto rb_text = player_c.rb_cached;
    // if (rb_text == "")
    //   rb_text = "RMB";
    // if (ability_c.ability_1_pressed && ability_c.ability_2_pressed)
    {
      lb_text = "L";
      rb_text = "R";
    }

    auto font_size = (float)FontSizes::SIZE_13;
    auto* font = get_inter_font(r);
    ImGui::PushFont(font, font_size);
    const auto text_size1 = font->CalcTextSizeA(font_size, FLT_MAX, -1, lb_text.c_str());
    const auto text_size2 = font->CalcTextSizeA(font_size, FLT_MAX, -1, rb_text.c_str());
    const auto text_pos1 = ImVec2(im_screenspace1.x - 0.5f * text_size1.x, im_screenspace1.y - 0.5f * text_size1.y);
    const auto text_pos2 = ImVec2(im_screenspace2.x - 0.5f * text_size2.x, im_screenspace2.y - 0.5f * text_size2.y);

    engine::SRGBColour my_text_col_1 = my_text_col_dark;
    my_text_col_1.a = (int)(ability_c.ability_1_alpha * 255.0f);
    engine::SRGBColour my_text_col_2 = my_text_col_dark;
    my_text_col_2.a = (int)(ability_c.ability_2_alpha * 255.0f);
    draw_list->AddText(font, font_size, text_pos1, convert_my_to_im(my_text_col_1), lb_text.c_str());
    draw_list->AddText(font, font_size, text_pos2, convert_my_to_im(my_text_col_2), rb_text.c_str());

    ImGui::PopFont();

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