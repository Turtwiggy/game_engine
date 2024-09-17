#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/gameover/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_gameover/components.hpp"

#include <imgui.h>

namespace game2d {

bool
centered_button(const std::string& label)
{
  const float alignment = 0.5f;
  ImGuiStyle& style = ImGui::GetStyle();

  const float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - size) * alignment;
  if (off > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

  return ImGui::Button(label.c_str());
};

void
update_ui_gameover_system(entt::registry& r)
{
  const auto& gameover = get_first_component<SINGLE_GameOver>(r);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  if (gameover.game_is_over) {

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoDecoration;
    // flags |= ImGuiWindowFlags_NoBackground;

    const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
    const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
    const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGui::Begin("Gameover", NULL, flags);
    ImGui::Text("%s", gameover.reason.c_str());

    if (centered_button("Back to ship.")) {
      move_to_scene_start(r, Scene::overworld_revamped, false);
      move_to_scene_additive(r, Scene::overworld_revamped);
    }

    /*
    if (centered_button("New Game"))
      create_empty<NewGameRequest>(r);
    */

    ImGui::End();
  }
}

} // namespace game2d