#include "trait_helpers.hpp"

#include "modules/system_traits/trait_components.hpp"

#include <algorithm>

namespace game2d {

bool
has_trait(entt::registry& r, const std::vector<Trait>& traits, const std::string& key)
{
  const auto find_trait = [&key](const Trait& t) { return t.key == key; };
  auto it = std::find_if(traits.begin(), traits.end(), find_trait);
  return (it != traits.end());
};

} // namespace game2d