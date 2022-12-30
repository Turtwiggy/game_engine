#pragma once

#include "components/app.hpp"

#include <entt/entt.hpp>

#include <string>

namespace game2d {

void
save(const entt::registry& registry, std::string path);

void
load(GameEditor& editor, Game& game, std::string path);

void
load_if_exists(entt::registry& registry, std::string path);

} // namespace game2d