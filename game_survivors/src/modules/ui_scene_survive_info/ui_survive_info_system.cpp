#include "ui_survive_info_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_autofire/autofire_helpers.hpp"
#include "modules/system_sprint/sprint_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade_dodge/upgrade_dodge_components.hpp"
#include "modules/system_upgrade_hp_regen/upgrade_hp_regen_components.hpp"
#include "modules/system_upgrade_xp_zone_size/upgrade_xp_zone_size_components.hpp"

#include <imgui.h>
#include <magic_enum.hpp>

namespace game2d {

void
update_ui_survive_info_system(entt::registry& r)
{
  return; // disabled system

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

    const auto fixture_e = get_fixture_by_tag(r, e, "fixture_player");
    const auto& hp_c = r.get<HealthComponent>(fixture_e);
    const std::string hp_label = std::format("HP: {:.2f}/{:.2f}", hp_c.hp, hp_c.max_hp);
    ImGui::SameLine();
    ImGui::Text("%s", hp_label.c_str());

    ImGui::NewLine();
    ImGui::Text("Upgrades...");
    const auto& upgrade_c = r.get_or_emplace<UpgradeComponent>(e);
    for (const auto& upgrade : upgrade_c.aquired_upgrades)
      ImGui::Text("%s", upgrade.c_str());

    ImGui::NewLine();
    ImGui::Text("Actor...");
    {
      auto& upgrades_c = r.get<StatModifierComponent>(e);

      const auto key_dodge = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_DODGE_CHANCE));
      const auto key_hp_max = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_HEALTH_MAX));
      const auto key_hp_regen = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_HEALTH_REGEN));
      const auto key_speed = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_SPEED));
      const auto key_stamina = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_STAMINA));
      const auto key_xp_zone_size = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_XP_ZONE_SIZE));

      const float val_dodge = r.get<ActorDodgeComponent>(e).dodge_percent;
      const float val_hp_max = 10; // todo: fix this, and fix this in the upgrade system
      const float val_hp_regen = r.get<ActorHealthRegenComponent>(e).hp_per_second;
      const float val_speed = r.get<ActorSpeedComponent>(e).current_speed;
      const float val_stamina = r.get<ActorStaminaComponent>(e).max_stamina;
      const float val_xp_zone_size = r.get<ActorXpZoneSizeComponent>(e).radius_meters;

      const float mod_dodge = upgrades_c.apply_modifiers(val_dodge, key_dodge);
      const float mod_hp_max = upgrades_c.apply_modifiers(val_hp_max, key_hp_max);
      const float mod_hp_regen = upgrades_c.apply_modifiers(val_hp_regen, key_hp_regen);
      const float mod_speed = upgrades_c.apply_modifiers(val_speed, key_speed);
      const float mod_stamina = upgrades_c.apply_modifiers(val_stamina, key_stamina);
      const float mod_xp_zone_size = upgrades_c.apply_modifiers(val_xp_zone_size, key_xp_zone_size);

      ImGui::Text("a_dodge_percent %f", mod_dodge);
      ImGui::Text("a_hp_max %0.2f", mod_hp_max);
      ImGui::Text("a_hp_regen %0.2f", mod_hp_regen);
      ImGui::Text("a_cur_speed %0.2f", mod_speed);
      ImGui::Text("a_xp_zone_rad %0.2f", mod_xp_zone_size);
      ImGui::Text("a_cur_stamina %0.2f", r.get<ActorStaminaComponent>(e).cur_stamina);
      ImGui::Text("a_mod_max_stamina %0.2f", mod_stamina);
    }

    const auto& weps_c = r.get<HasChildrenComponent>(e);
    for (const auto wep_e : weps_c.children) {

      const auto bul_def = get_bullet_def(r, e, wep_e);
      const auto wep_def = get_weapon_def(r, e, wep_e);

      // clang-format off
      ImGui::NewLine();
      ImGui::Text("Bullets...");
      ImGui::Text("b_bounce %i", bul_def.bounces); 
      ImGui::Text("b_size_x %f", bul_def.size.x); 
      ImGui::Text("b_speed %f", bul_def.speed); 
      ImGui::Text("b_damage %i", bul_def.damage); 
      ImGui::Text("b_pierce %i", bul_def.pierce); 
      ImGui::Text("b_knockback %i", bul_def.knockback_force); 
      ImGui::Text("b_crit_chance %0.2f", bul_def.crit_chance); 
      ImGui::Text("b_crit_damage %0.2f", bul_def.crit_damage);
      ImGui::Text("b_lifesteal %0.2f", bul_def.lifesteal);

      ImGui::NewLine();
      ImGui::Text("Weapon...");
      ImGui::Text("w_firerate %f", wep_def.fire_rate); 
      ImGui::Text("w_time_between_shots %f", r.get<WeaponFireRate>(wep_e).seconds_between_shots_max); 
      ImGui::Text("w_time_between_shots_cd %f", r.get<WeaponFireRate>(wep_e).seconds_between_shots_left); 
      ImGui::Text("w_projectiles %i", wep_def.projectiles); 
      ImGui::Text("w_spread %i", wep_def.spread_deg);
      ImGui::Text("w_bullets_max %i", wep_def.bullets_max); 
      ImGui::Text("w_bullets_cur %i", r.get<WeaponClipSize>(wep_e).bullets_cur); 
      ImGui::Text("w_reload_max %f", wep_def.reload_rate); 
      ImGui::Text("w_reload_cd %f", r.get<WeaponReloadRate>(wep_e).seconds_cur); 
      ImGui::Text("w_range %f", wep_def.range);
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