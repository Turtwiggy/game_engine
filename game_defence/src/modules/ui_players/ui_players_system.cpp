#include "ui_players_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/system_combat_bleed/combat_bleed_components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/system_tutorial/tutorial_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_players_system(entt::registry& r, const glm::ivec2 mouse_pos)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& anims = get_first_component<SINGLE_Animations>(r);

  auto init_e = get_first<SINGLE_Initiative>(r);
  if (init_e == entt::null)
    return;
  auto& init_c = r.get<SINGLE_Initiative>(init_e);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  // Top-Left of the screen
  const auto wh = ri.viewport_size_render_at;
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto padding = ImVec2(0, wh.y / 2.0f);
  const auto pos = ImVec2(viewport_pos.x + padding.x, viewport_pos.y + padding.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.5f));

  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("Mobs", NULL, flags);

  auto tut_e = get_first<SINGLE_TutorialMetrics>(r);
  if (tut_e != entt::null) {
    auto& tut_c = r.get<SINGLE_TutorialMetrics>(tut_e);
    ImGui::SeparatorText("Objective");
    ImGui::Text("%s", tut_c.objective.c_str());
    ImGui::Text("Turn: (%i/%i)", tut_c.turns_taken, tut_c.max_turns);
  }

  ImGui::SeparatorText("Units");

  int gridsize = 50;
  glm::ivec2 mouse_gp = { -1, -1 };
  const auto map_e = get_first<MapComponent>(r);
  if (map_e != entt::null) {
    const auto& map_c = get_first_component<MapComponent>(r);
    mouse_gp = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
    gridsize = map_c.tilesize;
  }

  for (int i = 0; const auto e : init_c.order) {
    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);
    // const auto& mob_c = r.get<Mob>(e);

    const auto& team_c = r.get<TeamComponent>(e);
    const auto gp = get_grid_position(r, e);
    const ImVec4 col_player_active = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
    const ImVec4 col_player_inactive = ImVec4(0.5f, 1.0f, 0.5f, 0.5f);
    const ImVec4 col_enemy_active = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
    const ImVec4 col_enemy_inactive = ImVec4(1.0f, 0.5f, 0.5f, 0.5f);
    const ImVec4 col_hovered = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 col_default = team_c.team == AvailableTeams::player ? col_player_inactive : col_enemy_inactive;

    // if i==0 , you're active
    if (i == 0) {
      col_default = team_c.team == AvailableTeams::player ? col_player_active : col_enemy_active;
      i++;
    }

    const bool hovered = gp == mouse_gp;
    if (hovered)
      col_default = col_hovered;

    const auto& hp_c = r.get<HealthComponent>(e);
    const auto* name_c = r.try_get<NameComponent>(e);
    const auto name_text = name_c->name;
    ImGui::TextColored(col_default, "%s (%i/%i)", name_c->name.c_str(), hp_c.hp, hp_c.max_hp);

    // Show which entity in the world is hovered.
    if (ImGui::IsItemHovered()) {
      Sprite s;
      s.sprite = "CURSOR_0";
      s.pos = engine::grid::grid_space_to_world_space_center(gp, gridsize);
      s.size = { gridsize, gridsize };
      s.z_idx = ZLayer::DEFAULT;
      // s.col = col; // default
      draw_sprite(r, s);
    }

    // ImGui::SameLine();
    // ImGui::Text("%i", r.get<InitiativeComponent>(e).initiative);

    // Show bleed info
    if (const auto& bleed_c = r.try_get<BleedComponent>(e)) {
      ImGui::SameLine();
      const auto bleed_red = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
      const auto text = std::format("{}", bleed_c->turns_left);
      const float text_width = ImGui::CalcTextSize(text.c_str()).x;
      const auto bleed_tooltip = std::format("Bleeding for {} turns", bleed_c->turns_left);
      const auto cursor = ImGui::GetCursorPos();

      ImGui::TextColored(bleed_red, "%s", text.c_str());
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", bleed_tooltip.c_str());

      // bleed icon
      const int tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
      const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
      ImVec2 tl{ 0.0f, 0.0f };
      ImVec2 br{ 1.0f, 1.0f };
      const auto result = convert_sprite_to_uv(r, "EFFECT_26_11");
      std::tie(tl, br) = result;

      ImGui::SetCursorPosX(cursor.x + text_width);
      ImGui::SetCursorPosY(cursor.y);
      ImGui::Image(im_id, { 16, 16 }, tl, br, bleed_red);
      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", bleed_tooltip.c_str());
    }

    // if (ImGui::Button("Take 1 damage")) {
    //   DamageEvent dmg_evt;
    //   dmg_evt.from = entt::null; // system
    //   dmg_evt.to = e;
    //   dmg_evt.type = DamageType::PURE;
    //   dmg_evt.amount = 1;
    //   const auto& evts = get_first_component<SINGLE_Events>(r);
    //   evts.dispatcher->trigger(dmg_evt);
    //   evts.dispatcher->update();
    // }

    ImGui::PopID();
    // ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Adjust 10.0f for spacing between buttons
  }

  // if (ImGui::Button("Clear##mobselected")) {
  //   const auto view = r.view<SelectedComponent>();
  //   r.remove<SelectedComponent>(view.begin(), view.end());
  // }

  ImGui::End();
}

} // namespace game2d