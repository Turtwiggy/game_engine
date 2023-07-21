#include "system.hpp"

// components
#include "colour/colour.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

// other lib headers
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

// TODO: look to improve this

void
game2d::update_ui_hierarchy_system(entt::registry& r)
{
  size_t entities = r.alive();
  static entt::entity selected_entity = entt::null;

  ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
    // Note: root is 1
    ImGui::Text("Total alive: %i", entities);

    // optimisation: paginate the shown entities
    static int SHOWING_INDEX = 0;
    const int MAX_TO_SHOW = 10;
    const int min_show = SHOWING_INDEX * MAX_TO_SHOW;
    const int max_show = (SHOWING_INDEX + 1) * MAX_TO_SHOW;
    std::string next_label = std::string("Next " + std::to_string(MAX_TO_SHOW));
    std::string prev_label = std::string("prev " + std::to_string(MAX_TO_SHOW));
    if (ImGui::Button(prev_label.c_str()))
      SHOWING_INDEX -= 1;
    ImGui::SameLine();
    if (ImGui::Button(next_label.c_str()))
      SHOWING_INDEX += 1;
    ImGui::Text("Showing between %i %i", min_show, max_show);

    const auto& view = r.view<TagComponent>();
    for (int i = 0; auto [entity, tag] : view.each()) {

      // optimisation; only show MAX_TO_SHOW in hierachy at one time
      bool valid = i >= min_show && i <= max_show;
      ++i;
      if (!valid)
        continue;

      game2d::imgui_draw_entity(r, tag.tag, entity, selected_entity);
    }

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

    if (r.all_of<PhysicsTransformComponent>(eid)) {
      PhysicsTransformComponent& ptc = r.get<PhysicsTransformComponent>(eid);
      imgui_draw_ivec2("Physics Pos (tl/tr): ", ptc.x_tl, ptc.y_tl);
      imgui_draw_ivec2("Physics Size: ", ptc.w, ptc.h);
    }

    if (r.all_of<SpriteComponent, SpriteColourComponent>(eid)) {
      SpriteComponent& sc = r.get<SpriteComponent>(eid);
      SpriteColourComponent& scc = r.get<SpriteColourComponent>(eid);

      // select sprite
      imgui_draw_ivec2("Sprite: ", sc.x, sc.y);
    }

    if (r.all_of<VelocityComponent>(eid)) {
      const VelocityComponent& c = r.get<VelocityComponent>(eid);
      float x = c.x;
      float y = c.y;
      imgui_draw_vec2("Vel: ", x, y);
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