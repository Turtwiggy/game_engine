#pragma once

#include <entt/fwd.hpp>

#include "engine/maths/maths.hpp"
#include "modules/combat/combat_core/components.hpp"

namespace game2d {

entt::entity
spawn_islander_unoccupied(entt::registry& r,
                          engine::RandomState& rnd,
                          const entt::entity island_e,
                          const std::string tag,
                          const AvailableTeams team,
                          const bool has_brain = false);

entt::entity
spawn_islander_unoccupied_edge(entt::registry& r,
                               engine::RandomState& rnd,
                               const entt::entity island_e,
                               const std::string tag,
                               const AvailableTeams team,
                               const bool has_brain = false);

} // namespace game2d