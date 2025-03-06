#include "ui_survive_weapon_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/system_autofire/autofire_helpers.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

#include <imgui.h>

#include <format>

namespace game2d {

const auto my_active_col = hex_to_srgb("#FFFFFF", 255);
const auto im_active_col = convert_my_to_im(my_active_col);

const std::vector<engine::SRGBColour> my_player_colours{
  hex_to_srgb("#E8DA58"), // yellow
  hex_to_srgb("#3D9B44"), // green
  hex_to_srgb("#00A8EB"), // blue
  hex_to_srgb("#D075CE"), // pink
};

const auto my_hp_bar_background_col = hex_to_srgb("#15171B");
const auto im_hp_bar_background_col = convert_my_to_im(my_hp_bar_background_col);

auto view_to_vector_of_ents = [](auto view) -> std::vector<entt::entity> {
  std::vector<entt::entity> vec;
  for (const auto& [e, comp_c] : view.each())
    vec.push_back(e);
  return vec;
};

void
update_ui_survive_weapon_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c);

  const auto grime_tex_id = search_for_texture_id_by_texture_path(ri_c, "grime_bar")->id;
  const auto monochrome_tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(grime_tex_id));
  const auto monochrome_im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(monochrome_tex_id));

  const float hp_bar_height = 18;
  const int max_num_weapons = 4;
  const int max_num_players = 4;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;

  static float distance_from_bottom_of_screen = 25;
  // imgui_draw_float("distance_x", distance_from_bottom_of_screen);

  const auto players_view = r.view<const PlayerComponent>();
  const auto players_e_vec = view_to_vector_of_ents(players_view);
  auto num_active_players = (int)players_e_vec.size();

  static bool debug_ui = false;
#if defined(_DEBUG)
  imgui_draw_bool("debug_weapons_ui", debug_ui);
  if (debug_ui) {
    // static int debug_players = 3;
    // imgui_draw_int("debug_players", debug_players);
    // static int debug_weapons = 2;
    // imgui_draw_int("debug_weapons", debug_weapons);
    // num_active_players = debug_players;
    // num_active_weapons = debug_weapons;
  }
#endif

  const auto set_window_pos = ImVec2{ 0, (float)ri_c.viewport_size_render_at.y - distance_from_bottom_of_screen };
  const auto set_window_size = ImVec2{ (float)ri_c.viewport_size_render_at.x, 69 };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0, 1.0 });
  ImGui::SetNextWindowSize(set_window_size);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  ImGui::Begin("WeaponSystemUI", nullptr, flags);

  const float rounding = 0.0f;

  const ImVec2 window_pos = ImGui::GetWindowPos();
  const ImVec2 window_size = ImGui::GetWindowSize();
  const auto player_ui_w = window_size.x / max_num_players; // always /4
  const auto player_ui_h = window_size.y;
  auto player_ui_tl = ImVec2{ window_pos.x, window_pos.y };
  auto player_ui_br = ImVec2{ window_pos.x + player_ui_w, window_pos.y + player_ui_h };

  // center ui...
  player_ui_tl.x += (window_size.x) * 0.5f - (num_active_players * player_ui_w * 0.5);
  player_ui_br.x += (window_size.x) * 0.5f - (num_active_players * player_ui_w * 0.5);

  for (int i = 0; i < max_num_players; i++) {

    const auto my_player_col = my_player_colours[i];
    const auto im_player_col = convert_my_to_im(my_player_col);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (i >= num_active_players) {
      // show disconnected ui?

      // background
      // const float inc = ((i + 1) / 4.0f);
      // auto p_max = ImVec2{ player_ui_tl.x + player_ui_w, player_ui_tl.y + player_ui_h };
      // draw_list->AddRectFilled(player_ui_tl, p_max, im_active_col, rounding);

      // move horizontally
      player_ui_tl.x += player_ui_w;
      player_ui_br.x += player_ui_w;
      continue;
    }

    const auto player_e = players_e_vec[i];
    const auto fixture_e = get_fixture_by_tag(r, player_e, "fixture_player");
    const auto& hp_c = r.get<HealthComponent>(fixture_e);
    const int hp = hp_c.hp;
    const int max_hp = hp_c.max_hp;
    const float hp_percent = hp / (float)max_hp;
    if (debug_ui) {
      // hp = 3;
      // max_hp = 100;
    }

    {
      ImVec2 p_max;

      // background
      // p_max = ImVec2{ window_pos.x + window_size.x, window_pos.y + window_size.y };
      // draw_list->AddRectFilled(window_pos, p_max, im_active_col, rounding);

      // health bar background
      p_max = ImVec2{ player_ui_tl.x + player_ui_w, player_ui_tl.y + hp_bar_height };
      draw_list->AddRectFilled(player_ui_tl, p_max, im_hp_bar_background_col, rounding);

      // health bar foreground
      p_max = ImVec2{ player_ui_tl.x + (hp_percent * player_ui_w), player_ui_tl.y + hp_bar_height };
      ImU32 foreground_col = IM_COL32(my_player_col.r, my_player_col.g, my_player_col.b, 100);
      draw_list->AddRectFilled(player_ui_tl, p_max, foreground_col, rounding);

      // health bar textured foreground
      ImVec2 tex_tl{ 0.0f, 0.0f };
      ImVec2 tex_br{ hp_percent * 1.0f, 1.0f };
      draw_list->AddImage(im_id, player_ui_tl, p_max, tex_tl, tex_br);

      // health line bold colour
      p_max = ImVec2{ player_ui_tl.x + player_ui_w, player_ui_tl.y + 2 };
      draw_list->AddRectFilled(player_ui_tl, p_max, im_player_col, rounding);

      // health line black line
      auto p_min = ImVec2{ player_ui_tl.x, player_ui_tl.y + hp_bar_height - 2 };
      p_max = ImVec2{ player_ui_tl.x + player_ui_w, player_ui_tl.y + hp_bar_height };
      draw_list->AddRectFilled(p_min, p_max, IM_COL32(0, 0, 0, 255), rounding);

      // health text
      const std::string text = std::format("HP: {} / {}", hp, max_hp);
      const auto text_size = ImGui::CalcTextSize(text.c_str());
      const auto bar_center = ImVec2{ player_ui_tl.x + player_ui_w * 0.5f, player_ui_tl.y + hp_bar_height * 0.5f };
      const auto center = ImVec2{ bar_center.x - text_size.x * 0.5f, bar_center.y - text_size.y * 0.5f };
      draw_list->AddText(center, IM_COL32(255, 255, 255, 255), text.c_str());
    }

    const auto* weapons_c = r.try_get<HasChildrenComponent>(players_e_vec[i]);
    if (!weapons_c)
      continue;
    const auto weapons_e_vec = weapons_c->children;
    const auto num_active_weapons = (int)weapons_e_vec.size();

    // gun section
    const auto w = (player_ui_w / max_num_weapons);
    const auto h = (player_ui_h - hp_bar_height);
    auto pos_l = ImVec2{ player_ui_tl.x, player_ui_tl.y + hp_bar_height };
    auto pos_r = ImVec2{ player_ui_tl.x + w, player_ui_tl.y + player_ui_h };

    for (int j = 0; j < max_num_weapons; j++) {
      // draw_list->AddRectFilled(pos_l, pos_r, IM_COL32(0, 255 * inc, 255 * inc, 255), rounding);

      const auto bar_padding_x = 6;      // padding from edges each side
      const auto bar_padding_bottom = 6; // padding from bottom of box
      const auto bar_h = 5;
      const auto bar_rounding = 6;
      const bool have_gun = j < num_active_weapons;
      if (!have_gun) {

        // draw a padlock or something
        ImVec2 im_tex_tl{ 0.0f, 0.0f };
        ImVec2 im_tex_br{ 1.0f, 1.0f };
        std::tie(im_tex_tl, im_tex_br) = convert_sprite_to_uv(r, "KEY_2");

        const float avail_w = pos_r.x - pos_l.x;
        const float avail_h = pos_r.y - pos_l.y - bar_h - bar_padding_bottom;
        const float icon_size = 32.0f;

        auto icon_p_min = pos_l;
        icon_p_min.x += (avail_w - icon_size) * 0.5; // center the icon x
        icon_p_min.y += (avail_h - icon_size) * 0.5; // center the icon y

        auto icon_p_max = icon_p_min;
        icon_p_max.x += icon_size;
        icon_p_max.y += icon_size;

        const auto faded_white = IM_COL32(255, 255, 255, 128);
        draw_list->AddImage(monochrome_im_id, icon_p_min, icon_p_max, im_tex_tl, im_tex_br, faded_white);

        // move horizontally
        pos_l.x += w;
        pos_r.x += w;
        continue;
      }

      // state...
      auto weapon_e = weapons_e_vec[j];
      const auto wep_def = get_weapon_def(r, player_e, weapon_e);
      const auto& weapon_clip_c = r.get<WeaponClipSize>(weapon_e);
      const auto& weapon_reload_c = r.get<WeaponReloadRate>(weapon_e);
      const float bullets_in_clip = weapon_clip_c.bullets_cur / (float)wep_def.bullets_max;
      const float reload_percent = weapon_reload_c.seconds_cur / (float)wep_def.reload_rate;
      float percent_to_display = 0.0f;
      // if we've got bullets, show your current bullets
      if (weapon_clip_c.bullets_cur > 0)
        percent_to_display = bullets_in_clip;
      // if no bullets, show reload time
      // (make it 1.0 to show bar as increasing while reloading)
      if (weapon_clip_c.bullets_cur == 0)
        percent_to_display = 1.0 - reload_percent;

      // draw weapon icon
      {
        const float avail_w = pos_r.x - pos_l.x;
        const float avail_h = pos_r.y - pos_l.y - bar_h - bar_padding_bottom;
        // const float icon_size = glm::min(avail_w, avail_h) * 0.5f;
        const float icon_size = 32.0f;

        ImVec2 im_tex_tl{ 0.0f, 0.0f };
        ImVec2 im_tex_br{ 1.0f, 1.0f };
        std::tie(im_tex_tl, im_tex_br) = convert_sprite_to_uv(r, "CROSSBOW_37_5");

        auto icon_p_min = pos_l;
        icon_p_min.x += avail_w * 0.5;
        icon_p_min.x -= icon_size * 0.5; // center the icon x
        icon_p_min.y += avail_h * 0.5;
        icon_p_min.y -= icon_size * 0.5; // center the icon y

        auto icon_p_max = icon_p_min;
        icon_p_max.x += icon_size;
        icon_p_max.y += icon_size;

        draw_list->AddImage(monochrome_im_id, icon_p_min, icon_p_max, im_tex_tl, im_tex_br);
        // draw_list->AddRectFilled(icon_p_min, icon_p_max, IM_COL32(255, 0, 0, 255));
      }

      // bar background.
      {
        // should probably check that pos_r.y - bar_h > h?
        const auto bar_l = ImVec2{ pos_l.x + bar_padding_x, pos_r.y - bar_h - bar_padding_bottom };
        const auto bar_r = ImVec2{ pos_r.x - bar_padding_x, pos_r.y - bar_padding_bottom };
        const auto my_gunbar_bg = hex_to_srgb("#5D5721");
        const auto im_gunbar_bg = convert_my_to_im(my_gunbar_bg);
        draw_list->AddRectFilled(bar_l, bar_r, im_gunbar_bg, bar_rounding);
      }

      // bar foreground
      // lerp between colours...
      {
        const auto bar_l = ImVec2{ pos_l.x + bar_padding_x, pos_r.y - bar_h - bar_padding_bottom };
        const auto bar_r = ImVec2{ pos_r.x - bar_padding_x, pos_r.y - bar_padding_bottom };
        const auto bar_w = bar_r.x - bar_l.x;
        const auto bar_r_adj = ImVec2{ bar_r.x - ((1.0f - percent_to_display) * bar_w), bar_r.y };
        const auto my_gunbar_bg = hex_to_srgb("#E8DA58");
        const auto im_gunbar_bg = convert_my_to_im(my_gunbar_bg);
        draw_list->AddRectFilled(bar_l, bar_r_adj, im_gunbar_bg, bar_rounding);
      }

      // move horizontally
      pos_l.x += w;
      pos_r.x += w;
    }

    // move horizontally
    player_ui_tl.x += player_ui_w;
    player_ui_br.x += player_ui_w;
  }

  ImGui::End();
  ImGui::PopStyleVar(4);
}

} // namespace game2d