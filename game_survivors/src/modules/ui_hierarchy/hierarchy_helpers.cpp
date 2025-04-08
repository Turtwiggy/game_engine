#include "pch.hpp"

#include "hierarchy_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/string/helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

std::optional<size_t>
get_category_idx(const std::vector<Category>& cs, const std::string& s)
{
  const auto it = std::find_if(cs.begin(), cs.end(), [&s](const auto& other) { return other.tag == s; });
  if (it == cs.end())
    return std::nullopt;
  return static_cast<size_t>(it - cs.begin());
};

std::map<entt::entity, std::vector<entt::entity>>
get_hierarchy(entt::registry& r)
{
  const auto& storage = r.storage<entt::entity>();

  std::map<entt::entity, std::vector<entt::entity>> parent_to_children;

  for (const std::tuple<entt::entity>& ent_tuple : storage.each()) {
    const auto& [e] = ent_tuple;

    const auto* tag = r.try_get<TagComponent>(e);
    if (!tag) {
      throw std::runtime_error("Non-tagged entity.");
    }

    if (const auto* has_parent = r.try_get<HasParentComponent>(e)) {
      const auto parent_e = has_parent->parent;
      //
      // Insert or init
      //
      if (parent_to_children.contains(parent_e))
        parent_to_children[parent_e].push_back(e);
      else
        parent_to_children[parent_e] = { e };

      continue;
    }

    // Already added by child
    if (parent_to_children.contains(e))
      continue;

    // init empty
    parent_to_children[e] = {};
  }

  return parent_to_children;
};

void
draw_hierarchy(entt::registry& r, const std::vector<Category>& categories, entt::entity& selected_e)
{
  GET_FIRST_OR_RETURN(SINGLE_DebugMenuBar, r, menu_e, menu_c);
  auto h_menu_state = gesert_menubar_state(menu_c, "Hierarchy");
  if (!h_menu_state.enabled)
    return;

  // Filter the Hierarchy
  static std::string filter = "";
  bool new_filter_input = false;
  std::string filter_input = filter;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  ImGui::Begin("Hierarchy", NULL, flags);

  const auto& storage = r.storage<entt::entity>();
  ImGui::Text("entt.in_use(): %zu", storage.in_use());

  // filter input textbox
  imgui_draw_string("Filter", filter_input);
  if (filter_input != filter) {
    filter = to_lower(filter_input);
    new_filter_input = true;
  }

  for (const auto& [parent, children] : get_hierarchy(r)) {
    if (!r.valid(parent) || parent == entt::null) {
      ImGui::Text("INVALID-ENTITY");
      continue;
    }

    const auto& parent_tag = r.get<TagComponent>(parent).tag;
    const auto parent_tag_lower = to_lower(parent_tag);

    // Limit: filter by input text
    if (parent_tag_lower.find(filter) == std::string::npos)
      continue;

    // limit entry by displayed categories
    const auto category_opt = get_category_idx(categories, parent_tag_lower);
    if (category_opt.has_value()) {
      const auto category_idx = category_opt.value();
      const auto category = categories[category_idx];
      if (!category.display)
        continue;
    };

    // const auto tree_str = std::format("eid: {}", static_cast<uint32_t>(parent));
    const auto tree_str = std::format("({}) {}##{}", children.size(), parent_tag, static_cast<uint32_t>(parent));

    bool node_open = ImGui::TreeNodeEx(tree_str.c_str());
    if (ImGui::IsItemClicked())
      selected_e = parent;

    if (node_open) {

      for (const auto& child_e : children) {
        const auto child_tag = r.get<TagComponent>(child_e).tag;
        const auto child_str = std::format("{}##", child_tag, static_cast<uint32_t>(child_e));

        ImGui::Selectable(child_str.c_str());
        if (ImGui::IsItemClicked())
          selected_e = child_e;
      }

      ImGui::TreePop();
    }
  }

  ImGui::End();
}

} // namespace game2d