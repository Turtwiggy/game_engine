#include "pch.hpp"

#include "system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/string/helpers.hpp"
#include "hierarchy_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_hierarchy_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_DebugMenuBar, r, menu_e, menu_c);

  const size_t entities = r.storage<entt::entity>().in_use();
  static entt::entity selected_entity = entt::null;
  static std::vector<Category> categories;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  // Update available Categories
  {
    for (const std::tuple<entt::entity>& ent_tuple : r.storage<entt::entity>().each()) {
      const auto& [e] = ent_tuple;

      const auto tag = to_lower(r.get<TagComponent>(e).tag);
      const auto category_opt = get_category_idx(categories, tag);
      if (category_opt.has_value())
        continue;

      // preset filters...

      if (tag.find("inventoryslot") != std::string::npos) {
        categories.push_back({ tag, false });
        continue;
      }

      // note: also filters out DataParticleEmitter
      if (tag.find("particle") != std::string::npos) {
        categories.push_back({ tag, false });
        continue;
      }

      if (tag.find("entity-pool-entity") != std::string::npos) {
        categories.push_back({ tag, false });
        continue;
      }

      if (tag.find("single_") != std::string::npos) {
        categories.push_back({ tag, false });
        continue;
      }

      if (tag.find("audiosource") != std::string::npos) {
        categories.push_back({ tag, false });
        continue;
      }

      if (tag.find("cooldowncomponent") != std::string::npos) {
        categories.push_back({ tag, false });
        continue;
      }

      categories.push_back({ tag });
    }

    // sort alphabetically
    std::sort(categories.begin(), categories.end(), [](const Category& a, const Category& b) { return a.tag < b.tag; });
  }

  auto cf_menu_state = gesert_menubar_state(menu_c, "Hierarchy Category Filter");
  if (cf_menu_state.enabled) {
    ImGui::Begin(cf_menu_state.name.c_str(), &cf_menu_state.enabled);

    // Display a filter for the categories
    for (auto& c : categories)
      imgui_draw_bool(c.tag, c.display);

    ImGui::End();
  }

  draw_hierarchy(r, categories, selected_entity);

  /*
    // optimisation: paginate the shown entities
    static int SHOWING_INDEX = 0;
    if (new_filter_input)
      SHOWING_INDEX = 0;
    const int MAX_TO_SHOW = 10;
    std::string next_label = std::string("Next " + std::to_string(MAX_TO_SHOW));
    std::string prev_label = std::string("prev " + std::to_string(MAX_TO_SHOW));
    if (ImGui::Button(prev_label.c_str()))
      SHOWING_INDEX -= 1;
    ImGui::SameLine();
    if (ImGui::Button(next_label.c_str()))
      SHOWING_INDEX += 1;

    const int min_show = SHOWING_INDEX * MAX_TO_SHOW;
    const int max_show = ((SHOWING_INDEX + 1) * MAX_TO_SHOW);
  }*/

  auto properties_menu_state = gesert_menubar_state(menu_c, "Properties");
  if (properties_menu_state.enabled) {
    // If an entity is selected draw it's properties
    ImGui::Begin(properties_menu_state.name.c_str(), NULL, ImGuiWindowFlags_NoFocusOnAppearing);
    if (selected_entity == entt::null) {
      ImGui::End();
      return; // make sure selected entity is valid
    }
    if (!r.valid(selected_entity)) {
      ImGui::End();
      return; // make sure selected entity is valid
    }

    const auto& eid = selected_entity;
    ImGui::Text("%u", static_cast<uint32_t>(eid));

    if (r.all_of<TagComponent>(eid)) {
      TagComponent& t = r.get<TagComponent>(eid);
      imgui_draw_string("Tag: ", t.tag);
    }

    if (r.all_of<TransformComponent>(eid)) {
      auto& transform = r.get<TransformComponent>(eid);
      imgui_draw_vec3("Render Pos: ", transform.position.x, transform.position.y, transform.position.z);
      imgui_draw_vec3("Render Size: ", transform.scale.x, transform.scale.y, transform.scale.z);
      imgui_draw_vec3(
        "Render Angle:", transform.rotation_radians.x, transform.rotation_radians.y, transform.rotation_radians.z);
    }

    if (auto* pb = r.try_get<PhysicsBodyComponent>(eid)) {
      const auto& pos_m = pb->body->GetPosition();
      float tmp_x = pos_m.x;
      float tmp_y = pos_m.y;
      imgui_draw_vec2("Physics Pos (in meters): ", tmp_x, tmp_y);

      const auto pos = glm::vec2{ pos_m.x * PIXELS_PER_METER, pos_m.y * PIXELS_PER_METER };
      tmp_x = pos.x;
      tmp_y = pos.y;
      imgui_draw_vec2("Physics Pos (in pixels): ", tmp_x, tmp_y);

      // const glm::vec2 size = get_size(r, eid);
      // tmp_x = size.x;
      // tmp_y = size.y;
      // imgui_draw_vec2("Physics Size: ", tmp_x, tmp_y);

      const auto& vel = pb->body->GetLinearVelocity();
      tmp_x = vel.x;
      tmp_y = vel.y;
      imgui_draw_vec2("Physics LinearVelocity", tmp_x, tmp_y);

      tmp_x = pb->body->GetAngle();
      imgui_draw_float("Physics Angle", tmp_x);

      tmp_x = pb->body->GetLinearDamping();
      imgui_draw_float("LinearDamping", tmp_x);

      auto body_type = pb->body->GetType();
      if (body_type == b2_kinematicBody)
        ImGui::Text("Physics Type is b2_kinematicBody.");
      if (body_type == b2_dynamicBody)
        ImGui::Text("Physics Type is b2_dynamicBody.");
      if (body_type == b2_staticBody)
        ImGui::Text("Physics Type is b2_staticBody.");

      for (auto* fixture = pb->body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
        bool is_sensor = fixture->IsSensor();
        imgui_draw_bool("Physics Fixture: is_sensor", is_sensor);
      }

      bool is_bullet = pb->body->IsBullet();
      imgui_draw_bool("Physics: is_bullet", is_bullet);
    }

    if (auto* sc = r.try_get<SpriteComponent>(eid)) {
      // select sprite
      imgui_draw_ivec2("Sprite: ", sc->tex_pos.x, sc->tex_pos.y);
    }

    // if (ImGui::BeginPopup("AddComponent")) {
    //   if (ImGui::MenuItem("PhysicsSizeComponent")) {
    //     r.emplace<PhysicsSizeComponent>(eid);
    //     ImGui::CloseCurrentPopup();
    //   }
    //   if (ImGui::MenuItem("ColourComponent")) {
    //     r.emplace<ColourComponent>(eid);
    //     ImGui::CloseCurrentPopup();
    //   }
    //   ImGui::EndPopup();
    // }

    ImGui::End();
  }
};

} // namespace game2d