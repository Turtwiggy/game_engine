#include "system.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"

#include "engine/physics/components.hpp"
#include "imgui.h"
#include "modules/system_overworld_change_direction/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_overworld_ship_label/components.hpp"
#include <box2d/b2_joint.h>
#include <box2d/b2_math.h>


namespace game2d {

struct Crew
{
  std::string name;
};

void
create_distance_joint(entt::registry& r, entt::entity a, entt::entity b)
{
  const auto& a_body = r.get<PhysicsBodyComponent>(a).body;
  const auto& b_body = r.get<PhysicsBodyComponent>(b).body;

  // Define a distance joint
  b2DistanceJointDef jointDef;
  jointDef.Initialize(a_body, b_body, a_body->GetPosition(), b_body->GetPosition());
  jointDef.collideConnected = true;
  jointDef.length = 50.0f;

  // the distance joint can also be made soft.
  // "springness". high value: stiff spring. low value: soft spring
  // soft spring values: 0.5 to 1.0
  // med spring: 1.0 to 5.0
  // high freq: 5 to 10+
  float frequencyHz = 1.0f;
  float dampingRatio = 1.0f; // "damping" usually 0.0 to 1.0.
  b2LinearStiffness(jointDef.stiffness, jointDef.damping, frequencyHz, dampingRatio, jointDef.bodyA, jointDef.bodyB);

  get_first_component<SINGLE_Physics>(r).world->CreateJoint(&jointDef);
};

void
update_ui_launch_crew_system(entt::registry& r)
{
  // bug: static: here is wrong
  static bool show_boardship_ui = false;

  const auto& view_reqs = r.view<RequestShowBoardShipButton>();
  for (const auto& [e, req_c] : view_reqs.each())
    show_boardship_ui = req_c.request;
  r.destroy(view_reqs.begin(), view_reqs.end());

  // ui: board ship button
  //
  if (show_boardship_ui) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
    const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);

    // window size
    const ImVec2 size = { 160, 160 * 9 / 16.0f };
    ImGui::SetNextWindowSizeConstraints(size, size);

    // position
    const float padding_y = size.y / 2.0f;
    const float center_x = viewport_pos.x + viewport_size_half.x;
    const float bottom_y = viewport_pos.y + ri.viewport_size_current.y - padding_y - 32.0f;
    const auto pos = ImVec2(center_x, bottom_y);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoDecoration;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Board Ship", NULL, flags);

    if (ImGui::Button("Board Ship", size)) {
      show_boardship_ui = false;
      move_to_scene_start(r, Scene::dungeon_designer);
      create_empty<RequestGenerateDungeonComponent>(r);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
  }

  //
  // Add a button to add a cargo box trailing behind player
  //

  const auto& view_cargobox_reqs = r.view<RequestSpawnCargoboxes>();
  int amount_to_spawn = 0;
  for (const auto& [e, req_c] : view_cargobox_reqs.each())
    amount_to_spawn += req_c.amount;
  r.destroy(view_cargobox_reqs.begin(), view_cargobox_reqs.end());

  //
  // fixed-joint approach (i.e. dragging cargo)
  //

  // for (int i = 0; i < amount_to_spawn; i++) {
  for (int i = 0; i < 0; i++) {
    if (i == 0) {
      const auto player_e = get_first<PlayerComponent>(r);
      const auto player_t = get_position(r, player_e);
      DataSpaceCargoActor desc;
      desc.pos = { player_t.x, player_t.y - 50 };
      const auto capsule_e = Factory_DataSpaceCargoActor::create(r, desc);
      r.emplace<ApplyForceInDirectionComponent>(capsule_e);

      const auto label = std::format("x{} Cargo", amount_to_spawn);
      r.emplace<SpaceLabelComponent>(capsule_e, SpaceLabelComponent{ label });
      create_distance_joint(r, player_e, capsule_e);
      break;
    }
  }

  //
  // cargodrone approach
  //

  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  if (amount_to_spawn > 0) {
    const auto player_t = get_position(r, player_e);

    DataSpaceCargoActor desc;
    desc.pos = { player_t.x, player_t.y - 50 };
    const auto capsule_e = Factory_DataSpaceCargoActor::create(r, desc);

    // set the capsule's dynamic target to the player ship
    r.emplace<DynamicTargetComponent>(capsule_e, player_e);

    ApplyForceToDynamicTarget cargopod;
    cargopod.speed = 100;
    cargopod.orbit = true;
    cargopod.reduce_thrusters = true;
    r.emplace<ApplyForceToDynamicTarget>(capsule_e, cargopod);

    const auto label = std::format("Your Drone. Cargo x{} ", amount_to_spawn);
    r.emplace<SpaceLabelComponent>(capsule_e, SpaceLabelComponent{ label });
  }
}

} // namespace game2d
