#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/events/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "resources/data.hpp"
#include "ui_survive_hp_bars.hpp"

namespace game2d {

const auto my_hp_bar_background_col = hex_to_srgb("#15171B");
const auto im_hp_bar_background_col = convert_my_to_im(my_hp_bar_background_col);

// e.g. 4 players with (0, 4, 2, 5) guns => returns 5
int
max_weapons_per_players(entt::registry& r, const std::vector<entt::entity>& players_e_vec)
{
  int n_weapons = 0;
  for (const auto player_e : players_e_vec) {
    auto& children_c = r.get<HasChildrenComponent>(player_e);
    int n_player_weps = 0;
    for (const auto child_e : children_c.children)
      if (auto* weapon_c = r.try_get<WeaponComponent>(child_e))
        n_player_weps++;
    n_weapons = glm::max((int)n_player_weps, n_weapons);
  }
  return n_weapons;
};

void
update_ui_survive_hp_bars_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& ri_c = SINGLE_RendererInfo::instance;
  const auto ui_scale = get_first_component<SINGLE_UIScaling>(r).scaling;

  const auto grime_tex_id = search_for_texture_id_by_texture_path(ri_c, "grime_bar")->id;
  const auto monochrome_tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  const auto im_id = (ImTextureID)(void*)(intptr_t)(grime_tex_id);
  const auto monochrome_im_id = (ImTextureID)(void*)(intptr_t)(monochrome_tex_id);

  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SMALL : FontSize::TEXT_SMALL_SCALED;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);

  const float hp_bar_height = font_size;
  const float hp_bar_width = 200.0f * font_scale;
  const float distance_from_bottom_of_screen = 15.0f;
  const float space_between_bars = 45.0f * font_scale;

  const auto players_e_vec = view_to_vector_of_ents<PlayerComponent>(r);
  const auto num_active_players = (int)players_e_vec.size();
  int n_weapons = max_weapons_per_players(r, players_e_vec);

  // static auto num_active_players = 1;
  // imgui_draw_int("debug_players", num_active_players);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoBackground;
  // flags |= ImGuiWindowFlags_AlwaysAutoResize;

  const auto set_window_pos = ImVec2{ 0, (float)ri_c.viewport_size_render_at.y - distance_from_bottom_of_screen };
  const auto set_window_size = ImVec2{ (float)ri_c.viewport_size_render_at.x, hp_bar_height * (2 + n_weapons) * ui_scale };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.0f, 1.0f });
  ImGui::SetNextWindowSize(set_window_size, ImGuiCond_Always);

  ImGui::Begin("HpBars", NULL, flags);
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();

  const auto center_x = window_tl.x + 0.5f * window_wh.x;
  auto first_tl_x = center_x;
  first_tl_x -= num_active_players * (0.5f * hp_bar_width);
  first_tl_x -= (num_active_players - 1) * (0.5f * space_between_bars);

  for (int i = 0; i < num_active_players; i++) {

    // data per bar.
    const auto player_e = players_e_vec[i];
    const auto player_idx = r.get<PlayerComponent>(player_e).idx;
    const auto children_c = r.get<HasChildrenComponent>(player_e);
    const auto fixture_e = get_fixture_by_tag(r, player_e, "fixture_player");
    const auto& hp_c = r.get<HealthComponent>(fixture_e);
    const float hp = hp_c.hp;
    const float max_hp = hp_c.max_hp;
    const float hp_percent = hp / max_hp;
    const auto player_col = default_player_colours[player_idx];

    // draw health bar!

    // bar bg.
    const auto full_bar_tl = ImVec2(first_tl_x, window_tl.y);
    const auto full_bar_br = ImVec2(first_tl_x + hp_bar_width, window_tl.y + hp_bar_height);
    draw_list->AddRectFilled(full_bar_tl, full_bar_br, im_hp_bar_background_col);

    // bar fg.
    const auto fg_col = IM_COL32(player_col.r, player_col.g, player_col.b, 0.5f * 255);
    const auto fg_bar_tl = ImVec2(first_tl_x, window_tl.y);
    const auto fg_bar_br = ImVec2(first_tl_x + hp_percent * hp_bar_width, window_tl.y + hp_bar_height);
    draw_list->AddRectFilled(fg_bar_tl, fg_bar_br, fg_col);

    // bar fg (textured).
    const auto tex_tl = ImVec2{ 0.0f, 0.0f };
    const auto tex_br = ImVec2{ hp_percent * 1.0f, 1.0f };
    draw_list->AddImage(im_id, fg_bar_tl, fg_bar_br, tex_tl, tex_br);

    // hp bar bold line
    const auto line_col = IM_COL32(player_col.r, player_col.g, player_col.b, 255);
    const auto line_tex_tl = ImVec2(first_tl_x, window_tl.y);
    const auto line_tex_br = ImVec2(first_tl_x + hp_bar_width, window_tl.y + 1);
    draw_list->AddRectFilled(line_tex_tl, line_tex_br, line_col);

    // hp bar black line
    const auto bottom_line_col = IM_COL32(20, 20, 20, 255);
    const auto bottom_line_tex_tl = ImVec2(first_tl_x, window_tl.y + hp_bar_height - 2);
    const auto bottom_line_tex_br = ImVec2(first_tl_x + hp_bar_width, window_tl.y + hp_bar_height);
    draw_list->AddRectFilled(bottom_line_tex_tl, bottom_line_tex_br, bottom_line_col);

    // health text.
    // const auto hp_text = std::format("{:.1f}/{:.1f}", hp, max_hp);
    // const auto text_size = font->CalcTextSizeA(font_size, FLT_MAX, -1, hp_text.c_str());
    // const auto bar_center = ImVec2(full_bar_tl.x + 0.5f * hp_bar_width, full_bar_tl.y + 0.5f * hp_bar_height);
    // const auto text_center = ImVec2(bar_center.x - (0.5f * text_size.x), bar_center.y - 0.5f * text_size.y);
    // draw_list->AddText(font, font_size, text_center, IM_COL32(255, 255, 255, 255), hp_text.c_str());

    //
    // weapon reload / bullet info
    //

    const auto weapons_e_vec = get_weapons(r, players_e_vec[i]);
    const auto num_active_weapons = (int)weapons_e_vec.size();

    const float space_between_hp_bar_and_weapon_info = 4;
    auto txt_tl = ImVec2{ full_bar_tl.x, full_bar_br.y + space_between_hp_bar_and_weapon_info };

    for (int j = 0; j < num_active_weapons; j++) {
      const auto wep_e = weapons_e_vec[j];

      // weapon reload/ammo bar
      const auto w = 100;
      const auto h = font_size;
      const auto pos_l = ImVec2(txt_tl.x, txt_tl.y);
      const auto pos_r = ImVec2(txt_tl.x + w, txt_tl.y + h);
      const auto bar_padding_x = 6; // padding from edges each side
      const auto bar_padding_y = 3;
      const auto bar_rounding = 6;

      // state.
      const auto wep_def = get_weapon_def(r, player_e, wep_e);
      const auto& weapon_clip_c = r.get<WeaponClipSize>(wep_e);
      const auto& weapon_reload_c = r.get<WeaponReloadRate>(wep_e);
      const float bullets_in_clip = weapon_clip_c.bullets_cur / (float)wep_def.bullets_max;
      const float reload_percent = glm::clamp(weapon_reload_c.seconds_cur / (float)wep_def.reload_rate, 0.0f, 1.0f);

      float percent_to_display = 0.0f;
      bool is_reloading = false;

      // if we've got bullets, show your current bullets
      if (weapon_clip_c.bullets_cur > 0)
        percent_to_display = bullets_in_clip;

      // if no bullets, show reload time
      // (make it 1.0-X to show bar as increasing while reloading)
      if (weapon_clip_c.bullets_cur == 0) {
        percent_to_display = 1.0 - reload_percent;
        is_reloading = true;
      }

      // bar background.
      {
        const auto bar_l = ImVec2{ pos_l.x + bar_padding_x, pos_l.y + bar_padding_y };
        const auto bar_r = ImVec2{ pos_r.x - bar_padding_x, pos_r.y - bar_padding_y };
        const auto my_gunbar_bg = hex_to_srgb("#5D5721");
        const auto im_gunbar_bg = convert_my_to_im(my_gunbar_bg);
        draw_list->AddRectFilled(bar_l, bar_r, im_gunbar_bg, bar_rounding);
      }

      // bar foreground
      // TODO: lerp between colours...
      {
        const auto bar_l = ImVec2{ pos_l.x + bar_padding_x, pos_l.y + bar_padding_y };
        const auto bar_r = ImVec2{ pos_r.x - bar_padding_x, pos_r.y - bar_padding_y };
        const auto bar_w = bar_r.x - bar_l.x;
        const auto bar_r_adj = ImVec2{ bar_r.x - ((1.0f - percent_to_display) * bar_w), bar_r.y };
        const auto my_gunbar_fg = hex_to_srgb("#E8DA58");
        const auto im_gunbar_fg = convert_my_to_im(my_gunbar_fg);
        const auto my_gunbar_reloading_fg = hex_to_srgb("#FF0000");
        const auto im_gunbar_reloading_fg = convert_my_to_im(my_gunbar_reloading_fg);
        draw_list->AddRectFilled(bar_l, bar_r_adj, is_reloading ? im_gunbar_reloading_fg : im_gunbar_fg, bar_rounding);
      }

      // weapon text
      auto txt_pos = ImVec2(pos_r.x, pos_l.y);
      if (r.all_of<Weapon_OnDiskData, WeaponLevelComponent>(wep_e)) {
        const auto& wep_ondiskdata = r.get<Weapon_OnDiskData>(wep_e);
        const auto& wep_level_c = r.get<WeaponLevelComponent>(wep_e);
        const auto text = std::format("[{}] {} Lv {}", j, wep_ondiskdata.name, wep_level_c.level);
        draw_list->AddText(font, font_size, txt_pos, IM_COL32(255, 255, 255, 255), text.c_str());
      }

      txt_tl.y += font_size; // move vertically
    }

    first_tl_x += hp_bar_width;
    first_tl_x += space_between_bars;
  }

  ImGui::End();
  ImGui::PopStyleVar(4);
}

} // namespace game2d
