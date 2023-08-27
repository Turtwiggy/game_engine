#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/gameover/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_gameover/components.hpp"
#include "renderer/components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_gameover_system(entt::registry& r)
{
  const auto& gameover = get_first_component<SINGLETON_GameOver>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  if (gameover.game_is_over) {

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiDockNodeFlags_AutoHideTabBar;
    flags |= ImGuiDockNodeFlags_NoResize;

    const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
    const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
    const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGui::Begin("GameOver!", NULL, flags);
    ImGui::Text("%s", gameover.reason.c_str());
    ImGui::Text("Thanks for playing.");

    if (ImGui::Button("Restart"))
      r.emplace<NewGameRequest>(r.create());

    if (ImGui::Button("Main Menu"))
      move_to_scene_start(r, Scene::menu); // hack: impl shouldnt be here

    ImGui::End();
  }
}

} // namespace game2d