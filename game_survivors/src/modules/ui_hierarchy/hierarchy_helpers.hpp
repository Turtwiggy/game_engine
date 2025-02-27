#pragma once

#include <entt/entt.hpp>

#include <optional>

namespace game2d {

struct Category
{
  std::string tag;
  bool display = true;
};

std::string
to_lower(const std::string& str);

std::optional<size_t>
get_category_idx(const std::vector<Category>& cs, const std::string& s);

void
draw_hierarchy(entt::registry& r, const std::vector<Category>& categories, entt::entity& selected_e);

} // namespace game2d