#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actor_brawler/actor_brawler_components.hpp"
#include "modules/combat/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/persistent/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/components.hpp"
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
  return;

  int brawlers = r.view<ActionKey>().size();

  if (brawlers > 1)
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

  if (centered_button("Brawl Again!")) {
    // move_to_scene_start(r, Scene::overworld_revamped, false);
    // move_to_scene_additive(r, Scene::overworld_revamped);

    move_to_scene_start(r, Scene::brawl);
  }

  ImGui::End();
}

} // namespace game2d