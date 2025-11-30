#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "pch.hpp"

#include "resources/data.hpp"
#include "ui_survive_hp_bars_worldspace_components.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_draw_text_helpers.hpp"
#include "ui_survive_hp_bars_worldspace_system.hpp"

namespace game2d {

void
update_ui_survive_hp_bars_worldspace_system(entt::registry& r, float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto view = r.view<UI_BufferComponent, const HealthComponent, const HasParentComponent>();
  for (const auto& [e, ui, hp_c, parent_c] : view.each()) {

    const auto parent_e = parent_c.parent;
    if (parent_e == entt::null || !r.valid(parent_e))
      continue;

    if (!r.all_of<PlayerComponent>(parent_e))
      continue;

    const auto parent_pos = get_position(r, parent_e);

    // tick down the timer
    std::for_each(ui.entries.begin(), ui.entries.end(), [dt](TimedEntry& entry) { entry.time_left -= dt; });

    // remove the out of time entries...
    const auto to_remove = [](const TimedEntry& entry) { return entry.time_left <= 0.0f; };
    ui.entries.erase(std::remove_if(ui.entries.begin(), ui.entries.end(), to_remove), ui.entries.end());

    // if (ui.entries.empty())
    //   continue;

    ui.time_since_damage += dt;
    float alpha = 255.0f;
    const float time_to_dissapear = 4.0f;
    const float percent_dissapear = ui.time_since_damage / time_to_dissapear;
    alpha = 255 - 255 * percent_dissapear;
    if (alpha <= 0.0f)
      continue;

    {
      const auto& ri = SINGLE_RendererInfo::instance;

      const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
      ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
      ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
      imgui_begin("overlay", ImGuiWindowFlags_NoInputs);
      auto* draw_list = ImGui::GetWindowDrawList();

      const auto boat_size = r.get<TransformComponent>(parent_e).scale;

      const float hp_bar_width = 1.6f * boat_size.x;
      const float hp_bar_height = 5.0f;

      const auto wsp = parent_pos + glm::vec2{ -0.5f * hp_bar_width, (0.5f * boat_size.y) + 12 };
      // const auto wsp = glm::vec2(text.worldspace_position.x, text.worldspace_position.y);
      const auto wsp_adj = glm::vec2{ wsp.x, wsp.y };
      const auto ss_pos = worldspace_to_screenspace(r, wsp_adj);
      // ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });

      const float cur_hp = (float)hp_c.hp;
      const float max_hp = (float)hp_c.max_hp;
      const float hp_percent = cur_hp / max_hp;

      float dmg_hp = 0;
      std::for_each(ui.entries.begin(), ui.entries.end(), [&dmg_hp](const TimedEntry& e) { dmg_hp += e.damage; });
      const float dmg_percent = dmg_hp / max_hp;

      // work out colours
      auto my_hp_bar_bg_col = my_hp_bar_background_col;
      my_hp_bar_bg_col.a = alpha;
      auto im_hp_bar_bg_col = convert_my_to_im(my_hp_bar_bg_col);
      const auto player_e = parent_c.parent;
      const auto& player_c = r.get<PlayerComponent>(player_e);
      const auto player_idx = player_c.idx;
      const auto player_col_idx = player_c.colour_idx;
      const auto player_col = default_player_colours[player_col_idx];
      const auto fg_col = IM_COL32(player_col.r, player_col.g, player_col.b, alpha);
      const auto dmg_col = IM_COL32(255, 255, 255, alpha);
      const auto line_col = IM_COL32(player_col.r, player_col.g, player_col.b, alpha);
      const auto bottom_line_col = IM_COL32(20, 20, 20, alpha);

      // bar bg
      const auto hp_bar_tl = ImVec2{ ss_pos.x, ss_pos.y };
      const auto hp_bar_br = hp_bar_tl + ImVec2{ hp_bar_width, hp_bar_height };
      draw_list->AddRectFilled(hp_bar_tl, hp_bar_br, im_hp_bar_bg_col);

      // bar fg
      const auto fg_bar_tl = hp_bar_tl;
      const auto fg_bar_br = ImVec2{ hp_bar_tl.x + hp_bar_width * hp_percent, hp_bar_br.y };
      draw_list->AddRectFilled(fg_bar_tl, fg_bar_br, fg_col);

      // draw the damage you've just recieved
      const auto hp_bar_injured_tl = ImVec2{ fg_bar_br.x, fg_bar_tl.y };
      const auto hp_bar_x = glm::min(fg_bar_br.x + hp_bar_width * dmg_percent, hp_bar_br.x);
      const auto hp_bar_injured_br = ImVec2{ hp_bar_x, hp_bar_br.y };
      draw_list->AddRectFilled(hp_bar_injured_tl, hp_bar_injured_br, dmg_col);

      // hp bar bold line
      const auto line_tex_tl = hp_bar_tl;
      const auto line_tex_br = ImVec2{ hp_bar_br.x, hp_bar_tl.y + 1 };
      draw_list->AddRectFilled(line_tex_tl, line_tex_br, line_col);

      // hp bar black line
      const auto bottom_line_tex_tl = ImVec2{ hp_bar_tl.x, hp_bar_br.y - 2 };
      const auto bottom_line_tex_br = hp_bar_br;
      draw_list->AddRectFilled(bottom_line_tex_tl, bottom_line_tex_br, bottom_line_col);

      ImGui::End();
    }
  }
}

} // namespace game2d