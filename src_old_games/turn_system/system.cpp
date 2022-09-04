
// header
#include "game/turn_system/system.hpp"

// components
#include "game/turn_system/components.hpp"

// temp
#include <imgui.h>
#include <magic_enum.hpp>

namespace game2d {

void
update_turn_system(entt::registry& registry){
  // auto& tm = registry.ctx().at<SINGLETON_TurnComponent>();

  // ImGui::Begin("TurnManager", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  // ImGui::Text("Turn: %i", tm.turn);
  // ImGui::Text("Phase: %s", std::string(magic_enum::enum_name(tm.phase)).c_str());

  // if (ImGui::Button("End Phase")) {
  //   if (tm.phase != TURN_PHASE::END)
  //     tm.phase = magic_enum::enum_cast<TURN_PHASE>(static_cast<int>(tm.phase) + 1).value();
  // }

  // if (ImGui::Button("End Turn")) {
  //   tm.turn += 1;
  //   tm.phase = TURN_PHASE::MOVE;
  // }

  // if (tm.phase == TURN_PHASE::MOVE) {
  //   ImGui::Text("MOVE: %i units", 0);
  // }
  // if (tm.phase == TURN_PHASE::SHOOT) {
  //   ImGui::Text("SHOOT: %i units", 0);
  // }
  // if (tm.phase == TURN_PHASE::CHARGE) {
  //   ImGui::Text("CHARGE: %i units", 0);
  // }
  // if (tm.phase == TURN_PHASE::FIGHT) {
  //   ImGui::Text("FIGHT: %i units", 0);
  // }
  // if (tm.phase == TURN_PHASE::MORALE) {
  //   ImGui::Text("MORALE: %i units", 0);
  // }

  // ImGui::End();
};

} // namespace game2d