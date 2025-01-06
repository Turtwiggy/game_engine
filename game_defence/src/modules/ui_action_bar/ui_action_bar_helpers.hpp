#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

bool
any_unit_is_moving(entt::registry& r);

bool
player_has_requested_action(entt::registry& r, entt::entity e);

void
update_remove_path_when_at_destination(entt::registry& r, entt::entity e);

void
action_button(entt::registry& r,
              entt::entity e,
              std::string action,
              std::string label,
              const bool enabled_cond = true,
              const bool action_cond = false);
void
display_actions_for_item(entt::registry& r, entt::entity e, entt::entity slot_e);

void
update_request_move_action(entt::registry& r, entt::entity e, const glm::ivec2 mouse_pos, bool request_action);
void
update_request_heal_action(entt::registry& r, entt::entity e);
void
update_request_combat_action(entt::registry& r, entt::entity e, const glm::ivec2 mouse_pos, bool request_action);

void
update_process_move_request(entt::registry& r, entt::entity e);
void
update_process_heal_request(entt::registry& r, entt::entity e);
void
update_process_combat_request(entt::registry& r, entt::entity e);
void
update_process_end_turn_request(entt::registry& r);

} // namespace game2d