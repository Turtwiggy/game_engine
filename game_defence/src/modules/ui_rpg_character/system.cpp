#include "system.hpp"

#include "components.hpp"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <string>
#include <vector>

namespace game2d {
using namespace std::literals;

void
update_ui_rpg_character_system(entt::registry& r)
{
  const auto convert_stat_to_proficiency = [](int val) -> int {
    if (val >= 19)
      return 4;
    if (val >= 18)
      return 3;
    if (val >= 16)
      return 2;
    if (val >= 13)
      return 1;
    if (val >= 9)
      return 0;
    if (val >= 6)
      return -1;
    if (val >= 4)
      return -2;
    if (val >= 3)
      return -3;
    if (val >= 2)
      return -4;
    if (val >= 1)
      return -5;
    return 0;
  };
  const auto append_proficiency_to_string = [&convert_stat_to_proficiency](const std::string& str,
                                                                           const int& stat) -> std::string {
    const int prof = convert_stat_to_proficiency(stat);
    return str + " (" + std::to_string(prof) + ")";
  };
  static bool first_time = true;

  ImGui::Begin("Create Character");
  static CharacterStats stats;
  {
    imgui_draw_string("Name", stats.name);
    ImGui::InputInt(append_proficiency_to_string("Str", stats.strength).c_str(), &stats.strength);
    ImGui::InputInt(append_proficiency_to_string("Dex", stats.dexterity).c_str(), &stats.dexterity);
    ImGui::InputInt(append_proficiency_to_string("Con", stats.constitution).c_str(), &stats.constitution);
    ImGui::InputInt(append_proficiency_to_string("Int", stats.intelligence).c_str(), &stats.intelligence);
    ImGui::InputInt(append_proficiency_to_string("Wis", stats.wisdom).c_str(), &stats.wisdom);
    ImGui::InputInt(append_proficiency_to_string("Cha", stats.charisma).c_str(), &stats.charisma);
  }
  if (ImGui::Button("Create##character")) {
    const auto e = r.create();
    r.emplace<CharacterStats>(e, stats);
    r.emplace<CharacterPrefab>(e);
  }
  ImGui::End();

  ImGui::Begin("Fights");

  ImGui::Text("Available Monsters");

  if (first_time) {
    first_time = false;
    const auto e = r.create();
    r.emplace<CharacterStats>(e, stats);
    r.emplace<CharacterPrefab>(e);
  }

  // Wombo Combo needs labels
  std::vector<std::string> labels;
  const auto& characters_view = r.view<CharacterStats, CharacterPrefab>();
  for (const auto& [e, stats, prefab] : characters_view.each())
    labels.push_back(stats.name);

  // User to select a monster
  static int item_current_idx = 0;
  WomboComboIn wombo_in(labels);
  wombo_in.label = "Select";
  wombo_in.current_index = item_current_idx;
  const auto out = draw_wombo_combo(wombo_in);
  item_current_idx = out.selected;

  // Active Fight
  static std::vector<entt::entity> active_fight;
  if (ImGui::Button(("Add Monster"))) {

    // probably a better way to do this
    for (int i = 0; const auto& [old_e, stats, prefab] : characters_view.each()) {
      if (i == item_current_idx) {
        const auto new_e = r.create();
        r.emplace<CharacterStats>(new_e, stats);
        r.emplace<InActiveFight>(new_e);
        break;
      }
      i++;
    }
  }

  if (ImGui::Button("Clear Fight"))
    active_fight.clear();

  static bool new_fight = false;
  if (ImGui::Button("Begin Fight"))
    new_fight = true;

  ImGui::Separator();
  ImGui::Text("Fight");
  ImGui::Separator();

  static engine::RandomState rnd;

  for (int i = 0; const auto& [e, stats, fighting] : r.view<CharacterStats, InActiveFight>().each()) {
    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    if (i > 0)
      ImGui::Separator();
    ImGui::Text("Monster: %s", stats.name.c_str());
    ImGui::Text("Initiative: %i", stats.initiative);
    ImGui::Text("HP: %i", stats.hp);
    ImGui::Text("XP: %i", stats.xp);

    if (ImGui::Button("Remove")) {
      r.destroy(e);

      // end
      i++;
      ImGui::PopID();
      continue;
    }

    // Initiative
    ImGui::SameLine();
    if (ImGui::Button("Roll Init"))
      stats.initiative = engine::rand_det_s(rnd.rng, 1, 21);

    // Damage
    ImGui::SameLine();
    if (ImGui::Button("Roll Dmg")) {
      const int damage = int(engine::rand_det_s(rnd.rng, 1, 12));
      r.emplace<DamageInstance>(r.create(), damage);
    }

    // XP
    ImGui::SameLine();
    if (ImGui::Button("Give XP"))
      stats.xp += 10;

    //
    // a drop zone here
    //
    std::string label = "MONSTER BODY. HP: "s + std::to_string(stats.hp);
    ImGui::Button(label.c_str());
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MONSTER_BODY")) {
        IM_ASSERT(payload->DataSize == sizeof(uint32_t));
        const auto payload_eid = *(const uint32_t*)payload->Data;
        const auto payload_e = static_cast<entt::entity>(payload_eid);

        // impl
        if (r.valid(payload_e)) {
          const auto instance = r.get<DamageInstance>(payload_e);
          stats.hp -= instance.amount;
        }
        r.destroy(payload_e);
      }
      ImGui::EndDragDropTarget();
    }

    // end
    i++;
    ImGui::PopID();
  }

  //
  // Items that you can drop in to player drop zones here
  //
  for (int i = 0; const auto& [e, instance] : r.view<DamageInstance>().each()) {
    auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    std::string label = "dmg:"s + std::to_string(instance.amount);

    if (i > 0)
      ImGui::SameLine();
    ImGui::Button(label.c_str(), ImVec2(60, 60));

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("MONSTER_BODY", &eid, sizeof(uint32_t));
      ImGui::EndDragDropSource();
    }

    ImGui::PopID();
    i++;
  }

  ImGui::End();
}

} // namespace game2d