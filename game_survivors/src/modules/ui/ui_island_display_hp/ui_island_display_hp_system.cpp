#include "pch.hpp"

#include "ui_island_display_hp_system.hpp"

#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_draw_text_helpers.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_ui_island_display_hp_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const int tilesize = SINGLE_Islands::instance.tilesize;
  std::unordered_set<entt::entity> player_island_eids;
  {
    const auto view = r.view<const MovementIslandComponent, const PlayerComponent, const TransformComponent>();
    for (const auto& [e, movement_c, player_c, t_c] : view.each()) {
      // const auto pos = glm::vec2{ t_c.position.x, t_c.position.y };
      // const auto pos_adj = pos - glm::vec2{ tilesize * 0.5f, tilesize * 0.5f };
      // const auto island_gp = engine::grid::worldspace_to_gridspace(pos_adj, tilesize);
      // const auto island_id = engine::encode_cantor_pairing_function(island_gp.x, island_gp.y);
      // const auto island_e = SINGLE_Islands::instance.id_to_island_eid.at(island_id);
      const auto island_e = movement_c.island_e;
      player_island_eids.emplace(island_e);
    }
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  const auto& ri = SINGLE_RendererInfo::instance;
  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  imgui_begin("overlay", ImGuiWindowFlags_NoInputs);
  const auto tl = ImGui::GetWindowPos();
  const auto wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();
  auto* font = get_inter_font(r, FontSize::TEXT_SIZE_16);

  const auto view = r.view<MovementIslandComponent, HealthComponent, TransformComponent>();
  for (const auto& [e, movement_c, hp_c, t_c] : view.each()) {

    const auto island_e = movement_c.island_e;
    if (const auto* hidden_c = r.try_get<IslandHiddenComponent>(island_e))
      continue; // dont show hp if island is hidden.

    const auto it = std::find(player_island_eids.begin(), player_island_eids.end(), island_e);
    if (it == player_island_eids.end())
      continue; // dont show hp if no players on your island.

    auto ws_pos = glm::vec2{ t_c.position.x, t_c.position.y };

    // offset y so you can see the thing you're interacting with
    ws_pos.y -= 1.25f * t_c.scale.y;

    const auto ss_pos = worldspace_to_screenspace(r, ws_pos);
    const auto im_ss_pos = ImVec2(ss_pos.x, ss_pos.y);
    const auto text = std::format("{}", hp_c.hp, hp_c.max_hp);
    const auto text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, text.c_str());

    const auto text_pos = center_text(font, text, im_ss_pos);
    draw_list->AddText(font, font->FontSize, text_pos, im_hp_col, text.c_str());

    // const auto& ri_c = SINGLE_RendererInfo::instance;
    // const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
    // const auto im_id = (ImTextureID)(void*)(intptr_t)tex_id;
    // const auto icon_size = 16;
    // const auto icon = "ICON_HEART";
    // const auto pos = ImVec2{ text_pos.x + text_size.x, text_pos.y };
    // const auto icon_p_tl = ImVec2{ pos.x, pos.y };
    // const auto icon_p_br = ImVec2{ pos.x + icon_size, pos.y + icon_size };
    // const auto [icon_uv_tl, icon_uv_br] = convert_sprite_to_uv(r, icon);
    // draw_list->AddImage(im_id, icon_p_tl, icon_p_br, icon_uv_tl, icon_uv_br, im_hp_col);
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d