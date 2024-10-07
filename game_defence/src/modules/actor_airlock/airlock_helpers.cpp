#include "airlock_helpers.hpp"

#include "engine/maths/grid.hpp"
#include "modules/map/components.hpp"

namespace game2d {

bool
vec_ivec2_contains_ivec2(const std::vector<glm::ivec2>& v, const glm::ivec2& t)
{
  return std::find(v.begin(), v.end(), t) != v.end();
};

} // namespace game2d