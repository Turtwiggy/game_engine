#include "pch.hpp"

#include "ui_combo_unlock_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/events/event_death/components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_combo_unlock/combo_unlock_helpers.hpp"
#include "modules/ui/ui_combo_unlock/ui_combo_unlock_system.hpp"
#include "modules/ui/ui_scene_survive_land_on_islands_popups/ui_land_on_island_popup_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_combo_unlock_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& ri_c = SINGLE_RendererInfo::instance;
  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  const float window_x_size = ri_c.viewport_size_render_at.x;
  const float window_y_size = ri_c.viewport_size_render_at.y;
  const auto set_window_size = ImVec2{ window_x_size, window_y_size };
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(set_window_size);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

  const auto ui_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_scale = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);
  ImGui::PushFont(font, font_scale);

  const auto monochrome_tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  const auto monochrome_im_id = (ImTextureID)(void*)(intptr_t)monochrome_tex_id;

  imgui_begin("ComboUnlockUI", ImGuiWindowFlags_NoInputs);
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();

  auto bg_col_transparent = my_window_border_col;
  bg_col_transparent.a = 100;

  for (const auto& [e, treasure_c, combo_c] : r.view<TreasureEnemyComponent, ComboUnlockComponent>().each()) {

    if (!combo_c.display)
      continue;

    const auto size = get_size(r, e);
    const auto wsp = get_position(r, e);
    const auto wsp_ss = worldspace_to_screenspace(r, wsp);
    const auto ss_pos_tl = ImVec2(wsp_ss.x, wsp_ss.y);

    // const float first_y_pos = ss_pos_tl.y + 0.5f * size.y;
    // const float first_y_neg = ss_pos_tl.y - 0.5f * size.y;
    // draw_list->AddRect({ first_x, first_y_neg }, { first_x + size.x, first_y_neg + size.y }, IM_COL32(255, 0, 0, 255));
    // draw_list->AddRect(ss_pos_tl, { ss_pos_tl.x + size.x, ss_pos_tl.y + size.y }, IM_COL32(0, 255, 0, 255));

    const auto pad = 4.0f;
    const float icon_size = 16.0f;
    const float display_w = 4 * (icon_size) + (2 * pad);
    const float display_h = (icon_size + 2.0f * pad);

    const auto popup_tl = ImVec2(ss_pos_tl.x - 0.5f * display_w, ss_pos_tl.y - 0.5f * display_h + size.y);
    const auto popup_br = ImVec2(popup_tl.x + display_w, popup_tl.y + display_h);
    const float first_x = popup_tl.x + pad;

    // draw a popup
    const auto im_bg_col_transparent = convert_my_to_im(bg_col_transparent);
    draw_list->AddRectFilled(popup_tl, popup_br, im_bg_col_transparent, 0.0f);
    draw_list->AddRect(popup_tl, popup_br, im_window_border_col);

    // draw the combo code.
    for (int i = 0; i < combo_c.unlock.size(); i++) {
      const auto val = combo_c.unlock[i];
      const auto spr = get_sprite_for_combodir(val);
      const auto icon_tl = ImVec2{ first_x + icon_size * i, popup_tl.y + pad };
      const auto icon_br = ImVec2{ icon_tl.x + icon_size, icon_tl.y + icon_size };
      const auto [icon_uv_tl, icon_uv_br] = convert_sprite_to_uv(r, spr);
      draw_list->AddImage(monochrome_im_id, icon_tl, icon_br, icon_uv_tl, icon_uv_br);
    }

    // draw the user input.
    for (int i = 0; i < combo_c.current.size(); i++) {
      const auto val = combo_c.current[i];
      const auto spr = get_sprite_for_combodir(val);
      const auto icon_tl = ImVec2{ first_x + icon_size * i, popup_tl.y + pad };
      const auto icon_br = ImVec2{ icon_tl.x + icon_size, icon_tl.y + icon_size };
      const auto [icon_uv_tl, icon_uv_br] = convert_sprite_to_uv(r, spr);
      draw_list->AddImage(monochrome_im_id, icon_tl, icon_br, icon_uv_tl, icon_uv_br, im_greenish);
    }

    // validate inputs; reset if an invalid input is entered.
    const auto& u = combo_c.unlock;
    const auto& i = combo_c.current;
    bool is_valid = true;
    is_valid &= i.size() <= u.size();
    is_valid &= std::equal(i.begin(), i.end(), u.begin()); // order must match
    if (!is_valid)
      combo_c.current.clear();

    // unlock the chest!
    if (combo_c.current == combo_c.unlock) {
      auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
      dead_c.dead.push_back(e);

      // Send death event.
      DeathEvent d_evt;
      d_evt.killed_by = entt::null;
      d_evt.dead = e; // parent not fixture
      auto& evts_c = SINGLE_Events::instance;
      evts_c.dispatcher->trigger(d_evt);
      evts_c.dispatcher->update();

      // prevent re-sending event due to unlock code matching
      r.remove<ComboUnlockComponent>(e);
    }
  }

  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar(3);
}

} // namespace game2d