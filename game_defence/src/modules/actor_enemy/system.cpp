#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "glm/glm.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat/helpers.hpp"
#include "modules/physics/components.hpp"
#include "modules/ui_economy/components.hpp"
#include "renderer/components.hpp"

#include <iostream>

namespace game2d {

void
update_enemy_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const float enemy_speed = 2.0f;

  // SET TARGET
  // Note: this should be closest target
  const auto& first_target = get_first<HearthComponent>(r);
  if (first_target == entt::null)
    return;
  const auto& first_target_transform = r.get<const TransformComponent>(first_target);

  const auto& ui_econ = get_first_component<SINGLETON_UiEconomy>(r);

  // Attach a class to a newly created enemy
  const auto& lifecycle = get_first_component<SINGLETON_EntityBinComponent>(r);
  for (int i = 0; i < lifecycle.created_this_frame.size(); i++) {
    const auto& e = lifecycle.created_this_frame[i];
    const auto& type = r.get<EntityTypeComponent>(e);
    if (type.type == EntityType::actor_enemy) {
      if (ui_econ.add_weapon) {
        create_class(r, e, type.type, ui_econ.weapon_to_add);
      } else {
        // create as spawner's class?
        create_class(r, e, type.type, Weapon::grunt);
      }
    }
  }

  const auto& view = r.view<const TransformComponent, VelocityComponent, EnemyComponent>();
  for (auto [entity, transform, vel, enemy] : view.each()) {
    // move towards player
    glm::ivec3 d = first_target_transform.position - transform.position;
    if (d.x == 0 && d.y == 0 && d.z == 0) // check same spot not clicked
      continue;

    glm::vec2 dir = glm::vec2(d.x, d.y);
    glm::vec2 n = dir;
    if (dir.x != 0.0f || dir.y != 0.0f)
      n = normalize(dir);

    glm::vec2 pdir{ n.x * enemy_speed, n.y * enemy_speed };
    vel.x = pdir.x;
    vel.y = pdir.y;
  }
}

// void
// enemy_ai::enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s)
// {
//   // calculate a vector ab
//   glm::vec2 ab = player.pos - obj.pos;
//   // calculate the point halfway between ab
//   glm::vec2 half_point = obj.pos + (ab / 2.0f);
//   // calculate the vector at a right angle
//   glm::vec2 normal = glm::vec2(-ab.y, ab.x);

//   // expensive(?) distance calc
//   float distance = glm::distance(obj.pos, player.pos);
//   float half_distance = distance / 2.0f;

//   // offset the midpoint via normal
//   float amplitude = half_distance * sin(glm::radians(obj.approach_theta_degrees));
//   half_point += (glm::normalize(normal) * amplitude);

//   glm::vec2 dir = glm::normalize(half_point - obj.pos);

//   move_along_vector(obj, dir, delta_time_s);
// };

} // namespace game2d