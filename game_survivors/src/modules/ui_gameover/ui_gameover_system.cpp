#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/system_gameover/gameover_components.hpp"
#include "ui_gameover_system.hpp"

namespace game2d {

void
update_ui_gameover_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c)

  for (const auto& [e, gameover_c] : r.view<GameOverComponent>().each()) {

    const ImVec2 ui_wh = { (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y };
    const ImVec2 ui_tl = { 0, 0 };
    const auto ui_center = ImVec2{ ui_tl.x + ui_wh.x * 0.5f, ui_tl.y + ui_wh.y * 0.5f };

    ImGui::SetNextWindowPos(ui_center, ImGuiCond_Always, { 0, 0 });
    ImGui::Begin("Gameover");

    ImGui::Text("Game is over!");

    if (gameover_c.win_condition) {
      ImGui::Text("You Won!");
    }
    if (!gameover_c.win_condition)
      ImGui::Text("You Lost!");

    if (ImGui::Button("Back to Menu"))
      move_to_scene_start(r, Scene::menu);

    ImGui::End();
  }
}

} // namespace game2d