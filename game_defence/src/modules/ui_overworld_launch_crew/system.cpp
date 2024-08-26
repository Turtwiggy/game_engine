#include "system.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/system_overworld_change_direction/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"

#include "imgui.h"
#include "modules/ui_overworld_ship_label/components.hpp"
#include <box2d/b2_joint.h>
#include <box2d/b2_math.h>

namespace game2d {

struct Crew
{
  std::string name;
};

// bool
// ButtonCenteredOnLine(const char* label, const ImVec2& bsize = ImVec2(0, 0), float alignment = 0.5f)
// {
//   ImGuiStyle& style = ImGui::GetStyle();
//   float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
//   float avail = ImGui::GetContentRegionAvail().x;
//   float off = (avail - size) * alignment;
//   if (off > 0.0f)
//     ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
//   return ImGui::Button(label, bsize);
// }

void
update_ui_launch_crew_system(entt::registry& r)
{
  ImGui::Begin("Launch Crew UI");

  static std::vector<Crew> crew{
    { "Captain" },  //
    { "Pilot" },    //
    { "Engineer" }, //
    { "Medic" },    //
    { "Doggo" },    //
    { "Soldier" },  //
  };
  static std::vector<Crew> crew_in_cargopod;

  // List crew in ship...
  ImGui::SeparatorText("Crew in ship...");
  for (int i = 0; const Crew& c : crew) {
    ImGui::Text("%s", c.name.c_str());
    ImGui::SameLine();

    std::string label = "add-to-cargo-pod##" + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      crew_in_cargopod.push_back(std::move(crew[i]));
      crew.erase(crew.begin() + i);
      continue;
    }
    i++;
  }

  // List crew in pods...
  ImGui::SeparatorText("Crew in cargo pods...");
  for (int i = 0; const auto& c : crew_in_cargopod) {
    ImGui::Text("%s", c.name.c_str());
    ImGui::SameLine();
    std::string label = "remove-from-cargo-pod##" + std::to_string(i);
    if (ImGui::Button(label.c_str())) {
      crew.push_back(std::move(crew_in_cargopod[i]));
      crew_in_cargopod.erase(crew_in_cargopod.begin() + i);
      continue;
    }
    i++;
  }

  if (crew_in_cargopod.size() > 0 && ImGui::Button("Launch your crew")) {
    const auto player_e = get_first<PlayerComponent>(r);
    const auto enemy_e = get_first<EnemyComponent>(r);
    const auto player_t = get_position(r, player_e);

    DataSpaceCapsuleActor desc;
    desc.pos = { player_t.x, player_t.y };
    const auto capsule_e = Factory_DataSpaceCapsuleActor::create(r, desc);

    // set the capsule's dynamic target to the enemy ship
    r.emplace<DynamicTargetComponent>(capsule_e, enemy_e);

    ApplyForceToDynamicTarget cargopod;
    cargopod.speed = 300;
    cargopod.orbit = false;
    cargopod.reduce_thrusters = false;
    r.emplace<ApplyForceToDynamicTarget>(capsule_e, cargopod);

    SpaceLabelComponent slabel;
    slabel.text = "Your Crew";
    r.emplace<SpaceLabelComponent>(capsule_e, slabel);

    // Goodbye crew!
    crew_in_cargopod.clear();
  }

  ImGui::End();

  //
  // ui: board ship button
  //

  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);

  // window size
  const ImVec2 size = { 160, 160 * 9 / 16.0f };
  ImGui::SetNextWindowSizeConstraints(size, size);

  // position
  const float padding_y = size.y / 2.0f;
  const float center_x = viewport_pos.x + viewport_size_half.x;
  const float bottom_y = viewport_pos.y + ri.viewport_size_current.y - padding_y;
  const auto pos = ImVec2(center_x, bottom_y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Board Ship", NULL, flags);
  ImGui::PopStyleVar();

  if (ImGui::Button("Board Ship", size)) {
    move_to_scene_start(r, Scene::dungeon_designer);
    create_empty<RequestGenerateDungeonComponent>(r);
  }

  ImGui::End();

  //
  // HACK: trial adding some cargo boxes behind your main ship
  //
  ImGui::Begin("Cargo Drone UI");

  if (ImGui::Button("Add cargo drone")) {
    const auto player_e = get_first<PlayerComponent>(r);
    const auto player_t = get_position(r, player_e);

    DataSpaceCargoActor desc;
    desc.pos = { player_t.x, player_t.y - 50 };
    const auto capsule_e = Factory_DataSpaceCargoActor::create(r, desc);

    r.emplace<ApplyForceInDirectionComponent>(capsule_e);

    SpaceLabelComponent slabel;
    slabel.text = "Cargo";
    r.emplace<SpaceLabelComponent>(capsule_e, slabel);

    // const auto& player_body = r.get<PhysicsBodyComponent>(player_e).body;
    // const auto& capsule_body = r.get<PhysicsBodyComponent>(capsule_e).body;

    // Define a distance joint
    // b2DistanceJointDef jointDef;
    // jointDef.Initialize(player_body, capsule_body, player_body->GetPosition(), capsule_body->GetPosition());
    // jointDef.collideConnected = true;
    // jointDef.length = 50.0f;

    // the distance joint can also be made soft.
    // "springness". high value: stiff spring. low value: soft spring
    // soft spring values: 0.5 to 1.0
    // med spring: 1.0 to 5.0
    // high freq: 5 to 10+
    // float frequencyHz = 1.0f;
    // float dampingRatio = 1.0f; // "damping" usually 0.0 to 1.0.
    // b2LinearStiffness(jointDef.stiffness, jointDef.damping, frequencyHz, dampingRatio, jointDef.bodyA, jointDef.bodyB);

    // get_first_component<SINGLE_Physics>(r).world->CreateJoint(&jointDef);
  }

  // Apply dragging force to the cargobox
  // if (cargo_e != entt::null) {
  //   const auto player_e = get_first<PlayerComponent>(r);
  //   const auto& player_body = r.get<PhysicsBodyComponent>(player_e).body;
  //   const auto& player_pos = player_body->GetPosition();
  //   const auto& player_vel = player_body->GetLinearVelocity();
  //   const auto player_dir = engine::angle_radians_to_direction(player_body->GetAngle());

  //   const auto& cargo_body = r.get<PhysicsBodyComponent>(cargo_e).body;
  //   const auto cargo_vel = cargo_body->GetLinearVelocity();
  //   const auto cargo_pos = cargo_body->GetPosition();

  //   // Calculate the desired spot for the cargo as directly behind the ship, plus some offset.
  //   auto desired_spot = player_pos;
  //   desired_spot -= { player_dir.x * 50.0f, player_dir.y * 50.0f };

  //   // test out desired spot
  //   // cargo_body->SetTransform(desired_spot, 0);

  //   const auto raw_dir_to_desired_spot = desired_spot - cargo_pos;
  //   const auto nrm_dir_to_desired_spot = engine::normalize_safe({ raw_dir_to_desired_spot.x, raw_dir_to_desired_spot.y });

  //   ApplyForceInDirectionComponent& force_c = r.get<ApplyForceInDirectionComponent>(cargo_e);
  //   const float force = 150.0f;
  //   force_c.tgt_vel = nrm_dir_to_desired_spot * force;

  //   // Apply the force
  //   // cargo_body->ApplyForceToCenter(force, true);

  //   // const b2Vec2 relative_vel = cargo_vel - player_vel;
  //   // float dragging_strength = 1.0f;

  //   // b2Vec2 force = -dragging_strength * relative_vel;
  //   // cargo_body->ApplyForceToCenter(force, true);

  //   //
  // }

  ImGui::End();
}

} // namespace game2d