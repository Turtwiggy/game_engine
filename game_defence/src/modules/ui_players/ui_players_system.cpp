#include "ui_players_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "imgui.h"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"

namespace game2d {

void
update_ui_players_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  auto init_e = get_first<SINGLE_Initiative>(r);
  if (init_e == entt::null)
    return;
  auto& init_c = r.get<SINGLE_Initiative>(init_e);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  // Top-Left of the screen
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto padding = ImVec2(100, 100);
  const auto pos = ImVec2(viewport_pos.x + padding.x, viewport_pos.y + padding.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("Mobs", NULL, flags);

  for (int i = 0; const auto e : init_c.order) {
    const auto eid = static_cast<uint32_t>(e);
    const auto& mob_c = r.get<Mob>(e);

    // sprite
    const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
    const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
    ImVec2 tl{ 0.0f, 0.0f };
    ImVec2 br{ 1.0f, 1.0f };
    const auto result = convert_sprite_to_uv(r, mob_c.renderable.sprite);
    std::tie(tl, br) = result;

    const std::string label = "player-portrait##" + std::to_string(eid);

    ImGui::ImageButton(label.c_str(), im_id, { 32, 32 }, tl, br);
    if (ImGui::IsItemClicked()) {
      const auto& selected_view = r.view<SelectedComponent>();
      r.remove<SelectedComponent>(selected_view.begin(), selected_view.end());
      r.emplace<SelectedComponent>(e);

      // move camera to newly selected unit
      const auto& follow_view = r.view<CameraLerpToTarget>();
      r.remove<CameraLerpToTarget>(follow_view.begin(), follow_view.end());
      r.emplace<CameraLerpToTarget>(e);

      const auto& input_view = r.view<InputComponent>();
      r.remove<InputComponent>(input_view.begin(), input_view.end());
      r.emplace<InputComponent>(e);

      const auto& keyboard_view = r.view<KeyboardComponent>();
      r.remove<KeyboardComponent>(keyboard_view.begin(), keyboard_view.end());
      r.emplace<KeyboardComponent>(e);
    }

    // const auto* init_c = r.try_get<InitiativeComponent>(e);
    // ImGui::SameLine();
    // ImGui::Text("%i.", init_c->initiative);

    const auto* name_c = r.try_get<NameComponent>(e);

    ImVec4 col_white = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    ImVec4 col_green = ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
    ImVec4 col_default = col_white;

    if (i == 0) {
      col_default = col_green;
      i++;
    }

    ImGui::SameLine();
    ImGui::TextColored(col_default, "%s", name_c->full_name.c_str());

    const auto* selected_c = r.try_get<SelectedComponent>(e);
    if (selected_c) {
      ImGui::SameLine();
      ImGui::Text("(X)");
    }
  }

  if (ImGui::Button("Clear##mobselected")) {
    const auto view = r.view<SelectedComponent>();
    r.remove<SelectedComponent>(view.begin(), view.end());
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d