#include "ui_survive_health_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "magic_enum.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_autofire/autofire_helpers.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

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

  const auto& view = r.view<PlayerComponent, DefaultColour>();

  // sort by player number
  // group.sort<PlayerComponent>([](const auto& a, const auto& b) { return a.idx < b.idx; });

  for (const auto [e, player_c, col_c] : view.each()) {

    const auto im_col =
      ImVec4{ col_c.colour.r / 255.0f, col_c.colour.g / 255.0f, col_c.colour.b / 255.0f, col_c.colour.a / 255.0f };
    const auto my_out_col = engine::SRGBColour(0.8f, 0.8f, 0.8f, 1.0f);
    const auto im_out_col = ImVec4{ my_out_col.r / 255.0f, my_out_col.g / 255.0f, my_out_col.b / 255.0f, 0.2f };
    const auto im_heart_col = im_col;
    const auto my_heart_col = col_c.colour;

    ImGui::SetCursorPosX(spacing.x + icon_size.y * 0);
    ImGui::SetCursorPosY(spacing.y + icon_size.y * player_c.idx);

    ImGui::Text("P%i", player_c.idx);

    const auto fixture_e = get_fixture_by_tag(r, e, "player");
    const auto& hp_c = r.get<HealthComponent>(fixture_e);
    std::string hp_label = std::format("HP: {}/{}", hp_c.hp, hp_c.max_hp);
    ImGui::SameLine();
    ImGui::Text("%s", hp_label.c_str());

    ImGui::NewLine();
    ImGui::Text("Upgrades...");
    const auto& upgrade_c = r.get_or_emplace<UpgradeComponent>(e);
    for (const auto& upgrade : upgrade_c.aquired_upgrades)
      ImGui::Text("%s", upgrade.c_str());

    const auto& weps_c = r.get<HasWeaponsComponent>(e);

    for (const auto wep_e : weps_c.weapons) {

      const auto bul_def = get_bullet_def(r, e, wep_e);
      const auto wep_def = get_weapon_def(r, e, wep_e);

      // clang-format off
      ImGui::NewLine();
      ImGui::Text("Weapon...");
      ImGui::Text("b_bounce %i", bul_def.bounces); 
      ImGui::Text("b_size_x %f", bul_def.size.x); 
      ImGui::Text("b_speed %i", bul_def.speed); 
      ImGui::Text("b_damage %i", bul_def.damage); 
      ImGui::Text("b_pierce %i", bul_def.pierce); 
      ImGui::Text("b_knockback %i", bul_def.knockback_force); 
      ImGui::Text("w_firerate %f", wep_def.firerate); 
      ImGui::Text("w_projectiles %i", wep_def.projectiles); 
      ImGui::Text("w_spread %i", wep_def.spread_deg);
      // clang-format on

      break; // show ui for only first weapon
    }

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