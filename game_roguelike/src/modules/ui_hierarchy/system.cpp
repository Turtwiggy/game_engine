#include "system.hpp"

// components
#include "colour/colour.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

// other lib headers
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

// TODO: look to improve this
void
game2d::update_ui_hierarchy_system(GameEditor& editor, Game& game)
{
  auto& registry = game.state;
  const auto& h_entity = registry.view<RootNode>().front();
  auto& hroot = registry.get<EntityHierarchyComponent>(h_entity);
  size_t root_entity_count = hroot.children.size();
  size_t entities = registry.alive();
  static entt::entity selected_entity = entt::null;

  ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  {
    // Note: root is 1
    ImGui::Text("Total alive: %i", entities);
    ImGui::Text("Under root: %i", root_entity_count);

    // UI optimisation:
    // paginate the shown entities
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

    // let root hierarchy entity be dropped on
    drop_accept_entity(registry, h_entity);

    // skip showing the root node, go to children
    for (int i = 0; const auto& child : hroot.children) {

      // optimisation; only show MAX_TO_SHOW in hierachy at one time
      bool valid = i >= min_show && i <= max_show;
      ++i;
      if (!valid)
        continue;

      if (registry.valid(child)) {
        const auto& tag = registry.get<TagComponent>(child).tag;
        imgui_draw_entity(editor, game, tag, child, selected_entity);
      } else {
        ImGui::Text("INVALID ENTITY");
      }
    }

    // If select anywhere in the window, make entity unselected
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
      selected_entity = entt::null;

    // Right click on menu
    if (ImGui::BeginPopupContextWindow(0, 1, false)) {
      if (ImGui::MenuItem("A wild uselss menu item appeared...!")) {
        // do nothing
      }
      ImGui::EndPopup();
    }
  }
  ImGui::End();

  //
  // If an entity is selected draw it's properties
  //

  ImGui::Begin("Properties", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  if (selected_entity != entt::null) {

    if (!registry.valid(selected_entity)) {
      ImGui::End();
      return; // make sure selected entity is valid
    }

    const auto& eid = selected_entity;

    if (registry.all_of<TagComponent>(eid)) {
      TagComponent& t = registry.get<TagComponent>(eid);
      imgui_draw_string("Tag: ", t.tag);
    }

    if (registry.all_of<TransformComponent>(eid)) {
      auto& transform = registry.get<TransformComponent>(eid);
      imgui_draw_ivec3("Pos: ", transform.position.x, transform.position.y, transform.position.z);
      imgui_draw_ivec3("Render Size: ", transform.scale.x, transform.scale.y, transform.scale.z);
      imgui_draw_vec3(
        "Render Angle:", transform.rotation_radians.x, transform.rotation_radians.y, transform.rotation_radians.z);
    }

    if (registry.all_of<PhysicsTransformComponent>(eid)) {
      PhysicsTransformComponent& ptc = registry.get<PhysicsTransformComponent>(eid);
      imgui_draw_ivec2("Physics Pos (tl/tr): ", ptc.x_tl, ptc.y_tl);
      imgui_draw_ivec2("Physics Size: ", ptc.w, ptc.h);
    }

    if (registry.all_of<SpriteComponent, SpriteColourComponent>(eid)) {
      SpriteComponent& sc = registry.get<SpriteComponent>(eid);
      SpriteColourComponent& scc = registry.get<SpriteColourComponent>(eid);

      // colour component
      ImGui::Text("Colour: ");
      ImGui::SameLine();
      engine::SRGBColour rgba = engine::LinearToSRGB(scc.colour);

      float colours[4] = { rgba.r / 255.0f, rgba.g / 255.0f, rgba.b / 255.0f, rgba.a };
      // colours[0] = glm::min(rgba.r, 255.0f);
      // colours[1] = glm::min(rgba.g, 255.0f);
      // colours[2] = glm::min(rgba.b, 255.0f);
      // colours[3] = glm::min(rgba.a, 1.0f);

      if (ImGui::ColorEdit4("##colour", colours)) {
        engine::SRGBColour result;
        result.r = static_cast<int>(colours[0] * 255.0f);
        result.g = static_cast<int>(colours[1] * 255.0f);
        result.b = static_cast<int>(colours[2] * 255.0f);
        result.a = colours[3];
        scc.colour = engine::SRGBToLinear(result);
      }

      // select sprite
      imgui_draw_ivec2("Sprite: ", sc.x, sc.y);
    }

    if (registry.all_of<VelocityComponent>(eid)) {
      const VelocityComponent& c = registry.get<VelocityComponent>(eid);
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
    //     registry.emplace<PhysicsSizeComponent>(eid);
    //     ImGui::CloseCurrentPopup();
    //   }
    //   if (ImGui::MenuItem("ColourComponent")) {
    //     registry.emplace<ColourComponent>(eid);
    //     ImGui::CloseCurrentPopup();
    //   }
    //   ImGui::EndPopup();
    // }
  }
  ImGui::End();
};