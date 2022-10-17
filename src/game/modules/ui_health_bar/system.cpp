#include "system.hpp"

#include "game/components/actors.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/renderer/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_ui_hp_bar(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  ImGuiWindowFlags window_flags = 0;
  // window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_NoNav;
  window_flags |= ImGuiWindowFlags_NoDocking;
  window_flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  // window_flags |= ImGuiWindowFlags_NoMove;
  // window_flags |= ImGuiWindowFlags_NoResize;
  // window_flags |= ImGuiWindowFlags_NoCollapse;
  // window_flags |= ImGuiWindowFlags_NoBackground;

  static bool open = true;
  ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiCond_FirstUseEver);
  ImGui::Begin("HP", &open, window_flags);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  auto view = r.view<HealthComponent, PlayerComponent>();
  for (auto [entity, health, player] : view.each()) {
    int& cur_hp = health.hp;
    int& max_hp = health.max_hp;
    float percent = cur_hp / (float)max_hp;

    // clang-format off
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail(); 
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;

    canvas_sz.x *= percent;
    canvas_sz.y = 12.0f;

    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    // clang-format on

    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(200, 25, 25, 255));

    ImGui::SameLine(0.0f);
    ImGui::Text("%i / %i", cur_hp, max_hp);
  }
  ImGui::End();
};

};