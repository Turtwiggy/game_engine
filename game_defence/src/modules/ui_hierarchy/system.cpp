#include "system.hpp"

// components
#include "imgui/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/components.hpp"

// other lib headers
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <magic_enum.hpp>

#include <utility>

namespace game2d {

std::string
to_lower(const std::string& str)
{
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
  return result;
}

void
update_ui_hierarchy_system(entt::registry& r)
{
  const size_t entities = r.storage<entt::entity>().in_use();
  static entt::entity selected_entity = entt::null;

  ImGuiWindowFlags flags = 0;
  // flags |= ImGuiDockNodeFlags_PassthruCentralNode;
  // flags |= ImGuiWindowFlags_NoMove;
  // flags |= ImGuiWindowFlags_NoTitleBar;
  // flags |= ImGuiWindowFlags_NoBackground;
  // flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  // flags |= ImGuiDockNodeFlags_AutoHideTabBar;
  // flags |= ImGuiDockNodeFlags_NoResize;

  ImGui::Begin("Hierarchy", NULL, flags);
  {
    ImGui::Text("Total alive: %i", entities);

    // Filter the Hierarchy
    static std::string filter = "";
    bool new_filter_input = false;
    std::string filter_input = filter;
    imgui_draw_string("Filter", filter_input);
    if (filter_input != filter) {
      filter = filter_input;
      new_filter_input = true;
    }

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

    if (filter == "") {

      for (int i = 0; const std::tuple<entt::entity>& ent_tuple : r.storage<entt::entity>().each()) {
        const auto& [e] = ent_tuple;

        // optimisation; only show MAX_TO_SHOW in hierachy at one time
        bool valid = i >= min_show && i < max_show;
        i++;
        if (!valid)
          continue;

        ImGui::Text("eid: %i", e);

        const auto* tag = r.try_get<TagComponent>(e);
        if (tag != nullptr) {
          ImGui::SameLine();
          imgui_draw_entity(r, tag->tag, e, selected_entity);
        } else
          ImGui::Text("Non-tagged entity");
      }

    } else {
      filter = to_lower(filter);

      //
      // Show the paginated filtered entities
      //
      std::vector<std::pair<entt::entity, std::string>> ents;
      for (const auto& [entity, tag] : r.view<TagComponent>().each()) {
        if (to_lower(tag.tag).find(filter) != std::string::npos)
          ents.push_back({ entity, tag.tag });
      }

      ImGui::Text("Showing between %i %i", min_show, max_show);
      for (int i = 0; const auto& [entity, tag] : ents) {
        if (i >= min_show && i <= max_show) {
          ImGui::Text("eid: %i", entity);
          ImGui::SameLine();
          imgui_draw_entity(r, tag, entity, selected_entity);
        }
        i++;
      }
    }

    // Display all the non-tagged entities
    int not_tagged = 0;
    for (const std::tuple<entt::entity>& ent_tuple : r.storage<entt::entity>().each()) {
      const auto& [e] = ent_tuple;
      const auto* tag = r.try_get<TagComponent>(e);
      if (tag == nullptr)
        not_tagged++;
    }
    ImGui::Text("Non-Tagged Entities: %i", not_tagged);

    // If select anywhere in the window, make entity unselected
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
      selected_entity = entt::null;

    // Right click on menu
    // ImGuiPopupFlags popup_flags = ImGuiPopupFlags_None;
    // if (ImGui::BeginPopupContextWindow("##hierarchycontext", popup_flags)) {
    //   if (ImGui::MenuItem("A wild uselss menu item appeared...!")) {
    //     // do nothing
    //   }
    //   ImGui::EndPopup();
    // }
  }

  ImGui::End();

  //
  // If an entity is selected draw it's properties
  //

  ImGui::Begin("Properties", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  if (selected_entity != entt::null) {

    if (!r.valid(selected_entity)) {
      ImGui::End();
      return; // make sure selected entity is valid
    }

    const auto& eid = selected_entity;

    if (r.all_of<TagComponent>(eid)) {
      TagComponent& t = r.get<TagComponent>(eid);
      imgui_draw_string("Tag: ", t.tag);
    }

    if (r.all_of<TransformComponent>(eid)) {
      auto& transform = r.get<TransformComponent>(eid);
      imgui_draw_ivec3("Pos: ", transform.position.x, transform.position.y, transform.position.z);
      imgui_draw_ivec3("Render Size: ", transform.scale.x, transform.scale.y, transform.scale.z);
      imgui_draw_vec3(
        "Render Angle:", transform.rotation_radians.x, transform.rotation_radians.y, transform.rotation_radians.z);
    }

    if (r.all_of<PhysicsTransformXComponent>(eid)) {
      PhysicsTransformXComponent& ptc = r.get<PhysicsTransformXComponent>(eid);
      imgui_draw_ivec2("PhysX (l, r): ", ptc.l, ptc.r);
    }
    if (r.all_of<PhysicsTransformYComponent>(eid)) {
      PhysicsTransformYComponent& ptc = r.get<PhysicsTransformYComponent>(eid);
      imgui_draw_ivec2("PhysY (b, t): ", ptc.b, ptc.t);
    }
    if (r.all_of<AABB>(eid)) {
      AABB& aabb = r.get<AABB>(eid);
      imgui_draw_ivec2("AABB (center): ", aabb.center.x, aabb.center.y);
      imgui_draw_ivec2("AABB (size): ", aabb.size.x, aabb.size.y);
    }
    if (r.all_of<VelocityComponent>(eid)) {
      const auto& c = r.get<VelocityComponent>(eid);
      float x = c.x;
      float y = c.y;
      imgui_draw_vec2("Vel: ", x, y);
    }

    if (auto* sc = r.try_get<SpriteComponent>(eid)) {
      // select sprite
      imgui_draw_ivec2("Sprite: ", sc->tex_pos.x, sc->tex_pos.y);
    }

    // Add component
    // if (ImGui::Button("Add component")) {
    //   ImGui::OpenPopup("AddComponent");
    // }

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
  }
  ImGui::End();
};

} // namespace game2d