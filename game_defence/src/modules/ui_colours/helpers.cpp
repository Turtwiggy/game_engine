#include "helpers.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "modules/ui_colours/components.hpp"

#include <algorithm>
#include <fmt/core.h>

namespace game2d {

engine::LinearColour
get_lin_colour_by_tag(entt::registry& r, const std::string& tag)
{
  const auto& c = get_first_component<SINGLE_ColoursInfo>(r);

  for (const auto& colour : c.colours) {
    const auto& tags = colour.tags;
    if (std::find(tags.begin(), tags.end(), tag) != std::end(tags)) {
      // colour contains the tag we're interested in!
      return engine::SRGBToLinear(colour.colour);
    }
  }

  // otherwise, return the colour
  fmt::println("warning: no colour tag found for: {}", tag);
  return engine::SRGBToLinear({ 1.0f, 1.0f, 1.0f, 1.0f });
}

engine::SRGBColour
get_srgb_colour_by_tag(entt::registry& r, const std::string& tag)
{
  const auto& c = get_first_component<SINGLE_ColoursInfo>(r);

  for (const auto& colour : c.colours) {
    const auto& tags = colour.tags;
    if (std::find(tags.begin(), tags.end(), tag) != std::end(tags)) {
      // colour contains the tag we're interested in!
      return colour.colour;
    }
  }

  // otherwise, return the colour
  fmt::println("warning: no colour tag found for: {}", tag);
  return engine::SRGBColour{ 1.0f, 1.0f, 1.0f, 1.0f };
}

} // namespace game2d