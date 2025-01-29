#include "trait_helpers.hpp"

#include "modules/system_traits/trait_components.hpp"

#include <algorithm>

namespace game2d {

bool
has_trait(entt::registry& r, const std::unordered_set<AquirableTrait>& traits_vec, const AquirableTrait& t)
{
  auto it = std::find(traits_vec.begin(), traits_vec.end(), t);
  return it != traits_vec.end();
};

bool
has_trait(entt::registry& r, const std::vector<AquirableTrait>& traits_vec, const AquirableTrait& t)
{
  auto it = std::find(traits_vec.begin(), traits_vec.end(), t);
  return it != traits_vec.end();
};

} // namespace game2d