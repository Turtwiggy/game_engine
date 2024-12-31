#include "ui_units_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/system_names/components.hpp"
#include "ui_units_components.hpp"
#include "ui_units_helpers.hpp"

#include <imgui.h>

namespace game2d {

std::vector<UnitType>
build_units_vector(entt::registry& r)
{
  std::vector<UnitType> state;

  const auto& view = r.view<const NameComponent, const UnitPersistentState>();
  for (const auto& [e, name_c, state_c] : view.each()) {
    UnitType to_serialize;
    to_serialize.name = name_c.name;
    to_serialize.active = state_c.active;
    to_serialize.permadead = state_c.permadead;
    state.push_back(to_serialize);
  }

  return state;
};

void
clear_units(entt::registry& r)
{
  // destroy all existing units..
  const auto& view = r.view<const NameComponent, const UnitPersistentState>();
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  for (const auto& [e, name_c, state_c] : view.each())
    dead.dead.emplace(e);
};

void
update_ui_units_system(entt::registry& r)
{
  ImGui::Begin("Units");

  // static std::string input_name = "Default";
  // imgui_draw_string("unit name", input_name);

  if (ImGui::Button("Add new unit...")) {
    UnitType unit;
    unit.name = "default";
    add_unit_to_entt(r, unit);
    save_units(r, build_units_vector(r));
  }

  if (ImGui::Button("SaveUnits")) {
    save_units(r, build_units_vector(r));
  }

  if (ImGui::Button("LoadUnits")) {
    clear_units(r);
    const auto units = load_units(r);
    std::for_each(units.begin(), units.end(), [&r](const auto& u) { add_unit_to_entt(r, u); });
  }

  bool state_changed = false;

  const auto& view = r.view<const NameComponent, UnitPersistentState>();
  for (const auto& [e, name_c, state_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    std::string name_label = std::format("{}", name_c.name.c_str());
    ImGui::SeparatorText(name_label.c_str());
    ImGui::Text("Dead: %i", state_c.permadead);
    ImGui::SameLine();
    ImGui::Text("Active: %i", state_c.active);

    ImGui::SameLine();
    if (ImGui::Button("Act")) {
      state_c.active = !state_c.active;
      state_changed = true;
    }

    /*
    if (auto* body_c = r.try_get<DefaultBody>(e))
      ImGui::Text("HasBody, size: %zu", body_c->body.size());
    if (auto* inv_c = r.try_get<DefaultInventory>(e))
      ImGui::Text("HasInventory, size: %zu", inv_c->inv.size());
    */

    ImGui::PopID();
  }

  ImGui::End();

  if (state_changed) {
    SDL_Log("state changed... saving");
    save_units(r, build_units_vector(r));
  }
}

} // namespace game2d