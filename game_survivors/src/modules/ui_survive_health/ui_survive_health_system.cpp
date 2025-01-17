#include "ui_survive_health_system.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll_bullet_enemy/event_coll_bullet_enemy_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_survive_health_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  ImVec2 tl{ 0.0f, 0.0f };
  ImVec2 br{ 1.0f, 1.0f };
  const ImVec2 icon_size{ 20, 20 };
  const ImVec2 spacing = { 8, 8 };
  const int distance_from_top_of_screen = 20.0f;

  ImGui::SetNextWindowPos({ 0, distance_from_top_of_screen }, ImGuiCond_Always, { 0.0f, 0.0f });

  ImGui::Begin("health", NULL, flags);

  const auto& group = r.group<PlayerComponent, HealthComponent, DefaultColour>();

  // sort by player number
  group.sort<PlayerComponent>([](const auto& a, const auto& b) { return a.idx < b.idx; });

  for (const auto [e, player_c, hp_c, col_c] : group.each()) {

    const auto im_col =
      ImVec4{ col_c.colour.r / 255.0f, col_c.colour.g / 255.0f, col_c.colour.b / 255.0f, col_c.colour.a / 255.0f };
    const auto my_out_col = engine::SRGBColour(0.8f, 0.8f, 0.8f, 1.0f);
    const auto im_out_col = ImVec4{ my_out_col.r / 255.0f, my_out_col.g / 255.0f, my_out_col.b / 255.0f, 0.2f };
    const auto im_heart_col = im_col;
    const auto my_heart_col = col_c.colour;

    ImGui::SetCursorPosX(spacing.x + icon_size.y * 0);
    ImGui::SetCursorPosY(spacing.y + icon_size.y * player_c.idx);

    ImGui::Text("P%i", player_c.idx);
    ImGui::SameLine();

    std::string hp_label = std::format("HP: {}/{}", hp_c.hp, hp_c.max_hp);
    ImGui::Text("%s", hp_label.c_str());

    ImGui::SameLine();
    ImGui::Text("DMG: %i", r.get<BulletDamage>(e).dmg);

    /*

    // draw health background
    for (int i = 0; i < hp_c.max_hp; i++) {
      ImGui::SetCursorPosX(spacing.x + icon_size.y * i);
      ImGui::SetCursorPosY(spacing.y + icon_size.y * player_c.idx);

      // worldspace health
      // Sprite s;
      // s.sprite = "ICON_HEART_OUTLINE";
      // s.col = my_outline_col;
      // s.pos = { i * 16, 0.0f * 16 };
      // s.size = glm::ivec2(icon_size.x, icon_size.y);
      // s.z_idx = ZLayer::FOREGROUND;
      // draw_sprite(r, s);

      const auto result = convert_sprite_to_uv(r, "ICON_HEART_OUTLINE");
      std::tie(tl, br) = result;
      if (i > 0)
        ImGui::SameLine();

      ImGui::Image(im_id, icon_size, tl, br, im_out_col);
    }

    // draw active health
    for (int i = 0; i < hp_c.hp; i++) {
      ImGui::SetCursorPosX(spacing.x + icon_size.y * i);
      ImGui::SetCursorPosY(spacing.y + icon_size.y * player_c.idx);

      const auto result = convert_sprite_to_uv(r, "ICON_HEART");
      std::tie(tl, br) = result;
      if (i > 0)
        ImGui::SameLine();
      ImGui::Image(im_id, icon_size, tl, br, im_heart_col);
    }
    */

    ImGui::NewLine();
  }

  ImGui::End();
}

} // namespace game2d