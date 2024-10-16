#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "modules/renderer/components.hpp"

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

struct Category
{
  std::string tag;
  bool display = true;
};

std::optional<size_t>
get_category_idx(std::vector<Category>& cs, const std::string& s)
{
  const auto it = std::find_if(cs.begin(), cs.end(), [&s](const auto& other) { return other.tag == s; });
  if (it == cs.end())
    return std::nullopt;
  return static_cast<size_t>(it - cs.begin());
};

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

  ImGui::Begin("Hierarchy Category Filter");

  // get all the categories of entity
  static std::vector<Category> categories;
  for (const std::tuple<entt::entity>& ent_tuple : r.storage<entt::entity>().each()) {
    const auto& [e] = ent_tuple;

    const auto* tag = r.try_get<TagComponent>(e);
    if (tag == nullptr) {
      continue;
    }

    const auto category_opt = get_category_idx(categories, tag->tag);
    if (!category_opt.has_value()) {

      // preset filters...

      if (tag->tag.find("InventorySlot") != std::string::npos) {
        categories.push_back({ tag->tag, false });
        continue;
      }

      // note: also filters out DataParticleEmitter
      if (tag->tag.find("particle") != std::string::npos) {
        categories.push_back({ tag->tag, false });
        continue;
      }

      categories.push_back({ tag->tag });
    }
  }

  // Display a filter for the categories
  for (auto& c : categories)
    imgui_draw_bool(c.tag, c.display);

  // sort alphabetically
  std::sort(categories.begin(), categories.end(), [](const Category& a, const Category& b) { return a.tag < b.tag; });

  ImGui::End();

  ImGui::Begin("Hierarchy", NULL, flags);
  {
    ImGui::Text("Total alive: %zu", entities);

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

        const auto* tag = r.try_get<TagComponent>(e);

        if (tag == nullptr)
          ImGui::Text("Non-tagged entity");
        else {

          // limit entry by displayed categories
          const auto category_opt = get_category_idx(categories, tag->tag);
          if (category_opt.has_value()) {
            const auto category_idx = category_opt.value();
            const auto category = categories[category_idx];
            if (!category.display) {
              i--; // wasnt displayed
              continue;
            }
          };

          imgui_draw_entity(r, tag->tag, e, selected_entity);
        }
      }
    }

    if (filter != "") {
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
          ImGui::Text("eid: %i", static_cast<uint32_t>(entity));
          ImGui::SameLine();
          imgui_draw_entity(r, tag, entity, selected_entity);
        }
        i++;
      }
    }

    // If select anywhere in the window, make entity unselected
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
      selected_entity = entt::null;
  }

  ImGui::End();

  // If an entity is selected draw it's properties
  //
  ImGui::Begin("Properties", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
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
    imgui_draw_vec3("Pos: ", transform.position.x, transform.position.y, transform.position.z);
    imgui_draw_vec3("Render Size: ", transform.scale.x, transform.scale.y, transform.scale.z);
    imgui_draw_vec3(
      "Render Angle:", transform.rotation_radians.x, transform.rotation_radians.y, transform.rotation_radians.z);
  }

  if (auto* pb = r.try_get<PhysicsBodyComponent>(eid)) {
    const auto& pos = pb->body->GetPosition();
    float tmp_x = pos.x;
    float tmp_y = pos.y;
    imgui_draw_vec2("Physics Pos: ", tmp_x, tmp_y);

    const glm::vec2 size = get_size(r, eid);
    tmp_x = size.x;
    tmp_y = size.y;
    imgui_draw_vec2("Physics Size: ", tmp_x, tmp_y);

    const auto& vel = pb->body->GetLinearVelocity();
    tmp_x = vel.x;
    tmp_y = vel.y;
    imgui_draw_vec2("Physics LinearVelocity", tmp_x, tmp_y);

    tmp_x = pb->body->GetAngle();
    imgui_draw_float("Physics Angle", tmp_x);
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
};

} // namespace game2d