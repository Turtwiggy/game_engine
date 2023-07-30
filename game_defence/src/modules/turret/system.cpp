#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/enemy/components.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

namespace game2d {

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt)
{
  const float dt = ms_dt / 1000.0f;
  const auto& enemies = r.view<const EnemyComponent, const TransformComponent>();
  const auto& turrets = r.view<TurretComponent, TransformComponent>();
  const auto& player = get_first<PlayerComponent>(r); // assume one player for now

  // hmm, this shouldnt be here, probably
  const auto& group_x = r.group<TransformComponent, SpriteComponent>();
  group_x.sort<TransformComponent>([&r](const auto& l, const auto& r) { return l.position.x < r.position.x; });
  const auto& group_y = r.group<TransformComponent, SpriteComponent>();
  group_y.sort<TransformComponent>([&r](const auto& l, const auto& r) { return l.position.y < r.position.y; });

  for (auto [t_entity, turret, t] : turrets.each()) {

    //
    // Get Closest Entity to turret
    //

    std::optional<int> idx_x;
    std::optional<int> idx_y;

    // a.x_tl < b.x_tl;
    auto it_x = std::find(group_x.begin(), group_x.end(), t_entity);
    if (it_x != group_x.end())
      idx_x = it_x - group_x.begin();

    // a.y_tl < b.y_tl;
    auto it_y = std::find(group_y.begin(), group_y.end(), t_entity);
    if (it_y != group_y.end())
      idx_y = it_y - group_y.begin();
    if (!idx_x.has_value())
      continue; // this turret missing from the sorted entity list?
    if (!idx_y.has_value())
      continue; // this turret missing from the sorted entity list?

    auto evaluate_closest = [&r, &t](const entt::entity& other, const EntityType& type) -> ClosestInfo {
      ClosestInfo oinfo;
      const auto& other_type = r.get<EntityTypeComponent>(other);

      // check type is of interest
      if (other_type.type != type)
        return oinfo; // early exit

      // calculate distance
      const auto& other_pos = r.get<TransformComponent>(other);
      auto d = t.position - other_pos.position;
      int d2 = d.x * d.x + d.y * d.y;

      // update info
      oinfo.e = other;
      oinfo.distance2 = d2;
      return oinfo;
    };

    ClosestInfo info;
    const EntityType type = EntityType::actor_enemy;

    // check left...
    for (int i = idx_x.value() - 1; i >= 0; i--) {
      const auto& other = group_x[i];
      auto oinfo = evaluate_closest(other, type);
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    // check right...
    for (int i = idx_x.value() + 1; i < group_x.size(); i++) {
      const auto& other = group_x[i];
      auto oinfo = evaluate_closest(other, type);
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    // check up... (y gets less)
    for (int i = idx_y.value() - 1; i >= 0; i--) {
      const auto& other = group_y[i];
      auto oinfo = evaluate_closest(other, type);
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    // check down... (y gets greater)
    for (int i = idx_y.value() + 1; i < group_y.size(); i++) {
      const auto& other = group_y[i];
      auto oinfo = evaluate_closest(other, type);
      if (oinfo.distance2 < info.distance2)
        info = oinfo;
    }

    if (info.e == entt::null)
      continue; // no enemy

    // rotate the turret to it !
    auto& e_transform = r.get<TransformComponent>(info.e);
    const auto& a = e_transform.position;
    const auto& b = t.position;
    glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
    glm::vec2 nrm_dir = raw_dir;
    if (raw_dir.x != 0.0f || raw_dir.y != 0.0f)
      nrm_dir = glm::normalize(raw_dir);
    float angle = engine::dir_to_angle_radians(nrm_dir);
    // std::cout << angle << "\n";

    // rotation is not handled properly, so its only visual atm
    t.rotation_radians.z = angle;

    // Spawn bullet logic

    if (turret.active)
      turret.time_between_bullets_left -= dt;

    if (turret.time_between_bullets_left < 0.0f) {

      const float bullet_speed = 200.0f;

      // create spawn request
      CreateEntityRequest req;
      req.type = turret.type_to_spawn;
      req.position = t.position;
      req.velocity = glm::ivec3(-nrm_dir.x * bullet_speed, -nrm_dir.y * bullet_speed, 0);
      auto e = r.create();
      r.emplace<CreateEntityRequest>(e, req);

      // reset timer
      turret.time_between_bullets_left = turret.time_between_bullets;
    }
  }
};

} // namespace game2d