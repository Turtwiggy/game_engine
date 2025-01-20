#pragma once

#include "hulls_components.hpp"

#include <entt/entt.hpp>

#include <optional>

namespace game2d {

SINGLE_Hulls
load_hulls(std::string directory);

std::optional<ShipHullData>
get_hull(const SINGLE_Hulls& hulls, const std::string& hull_name);

} // namespace game2d