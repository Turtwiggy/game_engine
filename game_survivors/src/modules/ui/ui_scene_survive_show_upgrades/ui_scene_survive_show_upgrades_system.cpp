#include "pch.hpp"

#include "ui_scene_survive_show_upgrades_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/sprites/helpers.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_scene_select/scene_select_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

const std::vector<std::pair<UpgradeableStat, std::string>> stat_display_order = {
  { UpgradeableStat::ACTOR_DODGE_CHANCE, "Dodge" }, //
  { UpgradeableStat::ACTOR_HEALTH_MAX, "Health" },  //
  { UpgradeableStat::ACTOR_HEALTH_REGEN, "Regen" }, //
  { UpgradeableStat::ACTOR_SPEED, "Speed" },        //
  // { UpgradeableStat::ACTOR_XP_ZONE_SIZE, "" },      //

  // { UpgradeableStat::WEAPON_PROJECTILES, "Shots" }, //
  // { UpgradeableStat::WEAPON_SPREAD, "Spread" },     //
  { UpgradeableStat::WEAPON_FIRERATE, "Firerate" },  //
  { UpgradeableStat::WEAPON_CLIP_SIZE, "Magazine" }, //
  { UpgradeableStat::WEAPON_RELOAD, "Reload" },      //
  // { UpgradeableStat::WEAPON_RANGE, "Range" },        //

  // { UpgradeableStat::BULLET_BOUNCE, "Bounce" },       //
  { UpgradeableStat::BULLET_CRIT_CHANCE, "Crit %" },   //
  { UpgradeableStat::BULLET_CRIT_DAMAGE, "Crit Mul" }, //
  { UpgradeableStat::BULLET_DAMAGE, "Damage" },        //
  { UpgradeableStat::BULLET_KNOCKBACK, "Knockback" },  //
  { UpgradeableStat::BULLET_LIFESTEAL, "Lifesteal" },  //
  { UpgradeableStat::BULLET_LIFETIME, "Lifetime" },    //
  { UpgradeableStat::BULLET_PIERCE, "Pierce" },        //
  // { UpgradeableStat::BULLET_SIZE, "Size" },           //
  { UpgradeableStat::BULLET_SPEED, "Speed" }, //

  // { UpgradeableStat::AREA_BEAMS_PER_WEAPON, "" }, //
  // { UpgradeableStat::AREA_SIZE, "" },             //
  { UpgradeableStat::AREA_STACK_DAMAGE, "Damage" },     //
  { UpgradeableStat::AREA_STACK_DURATION, "Duration" }, //
  // { UpgradeableStat::AREA_STACKS_PER_SHOT, "EL Mul" }, //
};

void
update_ui_survive_show_upgrades_system(entt::registry& r, float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& ri_c = SINGLE_RendererInfo::instance;
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize({ (float)400, (float)ri_c.viewport_size_render_at.y });

  imgui_begin("ShowUpgradesSystem", ImGuiWindowFlags_NoInputs);
  const auto ui_tl = ImGui::GetWindowPos();
  const auto ui_wh = ImGui::GetWindowSize();
  const auto ui_br = ImVec2{ ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };
  auto* draw_list = ImGui::GetWindowDrawList();
  auto* font = get_fingerpaint_font(r);
  const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom.png(GL_NEAREST)")->id;
  const auto custom_im_id = (ImTextureID)(void*)(intptr_t)custom_tex_id;

  // draw a red background
  // draw_list->AddRectFilled(ui_tl, ui_br, IM_COL32(50, 0, 0, 255));

  const auto ui_scale = get_first_component<SINGLE_UIScaling>(r);
  static float line_height = 32.0f;
  static float font_size = 18.0f;

#if defined(_DEBUG)
  // ImGui::Begin("Debug");
  // imgui_draw_float("font size", font_size, 0.5f);
  // imgui_draw_float("line height", line_height, 0.5f);
  // ImGui::End();
#endif

  const ImVec2 button_size = { line_height, line_height };
  const ImVec2 icon_sprite = { line_height, line_height };
  // note: X = (3 headers, 3 blanks)
  const float total_y = stat_display_order.size() * line_height + 3 * line_height;
  float first_y = 0.5f * (ri_c.viewport_size_render_at.y - total_y);

  // Dont do this per-stat. Just find the relevent data before the loop and do it once for all stats
  std::unordered_map<entt::entity, std::vector<entt::entity>> player_to_weapons_map;
  std::unordered_map<entt::entity, const StatModifierComponent*> player_to_stat_c_map;
  std::unordered_map<entt::entity, const PlayerComponent*> player_to_player_c_map;
  auto view = r.view<const PlayerBoatComponent, const PlayerComponent, const StatModifierComponent>();
  for (const auto& [player_e, player_boat_c, player_c, stat_c] : view.each()) {
    player_to_weapons_map[player_e] = get_weapons(r, player_e);
    player_to_stat_c_map[player_e] = &stat_c;
    player_to_player_c_map[player_e] = &player_c;
  }

  for (const auto& [stat, display_str] : stat_display_order) {
    const auto stat_str = std::string(magic_enum::enum_name(stat));
    const auto pretty_stat_str = make_stat_name_pretty_name(stat_str);

    if (stat == UpgradeableStat::ACTOR_DODGE_CHANCE) {
      ImVec2 text_pos = { ui_tl.x, first_y };
      draw_list->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), "Ship");
      first_y += 0.66f * line_height;
    }
    if (stat == UpgradeableStat::WEAPON_FIRERATE) {
      first_y += 0.34f * line_height;

      ImVec2 text_pos = { ui_tl.x, first_y };
      draw_list->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), "Weapons");
      first_y += 0.66f * line_height;
    }
    if (stat == UpgradeableStat::BULLET_CRIT_CHANCE) {
      first_y += 0.34f * line_height;

      ImVec2 text_pos = { ui_tl.x, first_y };
      draw_list->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), "Bullets");
      first_y += 0.66f * line_height;
    }
    if (stat == UpgradeableStat::AREA_STACK_DAMAGE) {
      first_y += 0.34f * line_height;

      ImVec2 text_pos = { ui_tl.x, first_y };
      draw_list->AddText(font, font_size, text_pos, IM_COL32(255, 255, 255, 255), "Elemental");
      first_y += 0.66f * line_height;
    }

    // draw the stat icon.
    const auto draw_icon = [&](std::string key) {
      const auto icon_key = to_upper(key);
      const auto [image_icon_tl, image_icon_br] = convert_sprite_to_uv(r, icon_key);
      auto icon_pos = ImVec2{ ui_tl.x, first_y };
      // icon_pos.x += 0.5f * (button_size.x - icon_sprite.x);
      // icon_pos.y += 0.5f * (button_size.x - icon_sprite.y);
      ImGui::SetCursorScreenPos(icon_pos);
      ImGui::Image(custom_im_id, icon_sprite, image_icon_tl, image_icon_br);
    };
    draw_icon("ICON_" + stat_str + "_CENTERED");

    // draw text
    ImVec2 text_tl = { ui_tl.x + icon_sprite.x, first_y + 0.5f * (line_height - font_size) };
    draw_list->AddText(font, font_size, text_tl, im_white, display_str.c_str());

    float first_x = 110;

    // Draw how many upgrades of this stat the player has.
    // for (const auto& [e, boat_c, player_c, stat_c] : view.each()) {
    for (int i = 0; i < 4; i++) {
      auto player_e = get_player_e_from_idx(r, i);
      if (player_e == entt::null)
        continue;
      const auto* player_c = player_to_player_c_map[player_e];
      const auto* stat_c = player_to_stat_c_map[player_e];

      const float inset_y = 0.0f;
      const float block_w = 5;
      const float block_h = 16;
      const float pad_x = 4.0f;
      const auto col = default_player_colours_im[player_c->colour_idx];

      // work out how many of this upgrade you have
      const auto weapon_e = player_to_weapons_map[player_e][0]; // assumes everyone is using 1 weapon
      const auto cmp = [&](const auto& modifier) { return modifier->stat == stat_str; };
      const auto& weapon_stat_c = r.get<StatModifierComponent>(weapon_e);
      const int actor_occurances = std::count_if(stat_c->modifiers.begin(), stat_c->modifiers.end(), cmp);
      const int weapon_occurances = std::count_if(weapon_stat_c.modifiers.begin(), weapon_stat_c.modifiers.end(), cmp);
      const int n_stat = actor_occurances + weapon_occurances;

      // draw some blocks.
      for (int j = 0; j < n_stat; j++) {
        ImVec2 b_min = { ui_tl.x + first_x, first_y + 0.5f * (line_height - block_h) };
        ImVec2 b_max = { b_min.x + block_w, b_min.y + block_h };
        draw_list->AddRectFilled(b_min, b_max, col);

        first_x += block_w + pad_x; // spacer between blocks
      }

      if (n_stat > 0)
        first_x += block_w; // spacer between player info
    }

    first_y += line_height; // move vertically
  }

  ImGui::End();
}

} // namespace game2d