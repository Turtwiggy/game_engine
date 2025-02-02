#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/persistent/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/ui_scene_main_menu/components.hpp"

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
  // Work out if game is over?
  return; // system disabled for the moment

  int enemies = 0;
  int players = 0;
  for (const auto& [e, team_c] : r.view<TeamComponent>().each()) {
    if (team_c.team == AvailableTeams::enemy)
      enemies++;
    if (team_c.team == AvailableTeams::player)
      players++;
  }

  bool gameover_win = false;
  gameover_win |= enemies == 0;

  bool gameover_loss = false;
  gameover_loss |= players == 0;

  if (!gameover_win && !gameover_loss)
    return;

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("Gameover", NULL, flags);

  // if (gameover_win) {
  //   ImGui::Text("You win!");
  //   ImGui::Text("All enemies dead!");
  // }

  // if (gameover_loss) {
  //   ImGui::Text("You lose!");
  //   ImGui::Text("All players dead!");
  // }

  if (centered_button("To Menu")) {
    // move_to_scene_start(r, Scene::overworld_revamped, false);
    // move_to_scene_additive(r, Scene::overworld_revamped);

    if (gameover_win) {
      const auto info_e = get_first<MenuToNextSceneInfo>(r);
      if (info_e != entt::null) {
        auto& info_c = r.get<MenuToNextSceneInfo>(info_e);
        save_level(r, info_c.level, true);
      }
    }

    move_to_scene_start(r, Scene::menu);
  }

  ImGui::End();
}

} // namespace game2d