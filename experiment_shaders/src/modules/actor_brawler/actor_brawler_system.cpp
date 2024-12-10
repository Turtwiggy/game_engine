#include "actor_brawler_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_brawler/actor_brawler_components.hpp"
#include "modules/actor_brawler/actor_brawler_helpers.hpp"
#include "modules/combat_scale_on_hit/helpers.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/screenshake/components.hpp"

#include <glm/common.hpp>
#include <glm/fwd.hpp>

namespace game2d {

void
update_actor_brawler_system(entt::registry& r, const float dt)
{
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  auto& evts_c = get_first_component<SINGLE_Events>(r);

  const auto& brawlers_view = r.view<const ActionKey, const PhysicsBodyComponent, TransformComponent, Brawler>();
  for (const auto& [e, action_c, body_c, transform_c, brawler_c] : brawlers_view.each()) {

    auto& offset_c = r.get_or_emplace<TransformOffset>(e);
    bool is_ai = r.try_get<BrawlerAI>(e);
    bool wants_to_punch = r.try_get<WantsToPunch>(e) != nullptr;
    const bool input_pressed = get_key_down(input_c, action_c.key);
    const bool input_released = get_key_up(input_c, action_c.key);
    const bool input_held = get_key_held(input_c, action_c.key);

    if (input_released) {
      brawler_c.key_held_s = 0.0f;
      remove_if_exists<Blocking>(r, e);
    }

    if (input_held) {
      brawler_c.key_held_s += dt;

      // if (brawler_c.allowed_to_block)
      r.emplace_or_replace<Blocking>(e);
    }

    if (brawler_c.key_held_s >= brawler_c.do_thing_threshold) {
      // SDL_Log("do something...");
      // brawler_c.key_held_s = 0.0f;

      SDL_Log("block ran out...");
      remove_if_exists<Blocking>(r, e);
    }

    if (is_ai && wants_to_punch)
      r.remove<WantsToPunch>(e);

    if ((is_ai && wants_to_punch) || (!is_ai && input_pressed)) {

      // apply a force in the direction of your nearest enemy...
      const auto nearest_target = query_aabb_get_nearest_target(r, e);
      if (nearest_target != entt::null && e != nearest_target) {
        const auto pa = get_position(r, e);
        const auto pb = get_position(r, nearest_target);
        const auto raw_dir = pb - pa;
        const auto nrm_dir = engine::normalize_safe(raw_dir);
        const auto d2 = raw_dir.x * raw_dir.x + raw_dir.y * raw_dir.y;
        // SDL_Log("D2: %f", d2);

        // const float impulse = 1000;
        // body_c.body->ApplyLinearImpulse(b2Vec2(impulse, impulse), { pa.x, pa.y }, true);

        float str = 100.0f;
        body_c.body->SetLinearVelocity({ nrm_dir.x * str, nrm_dir.y * str });

        // apply a offset to the transform
        offset_c.max_offset = 5.0f * nrm_dir;
        offset_c.t = 0.0f;

        // if distance is in range
        if (d2 < 1500.0f) {

          DamageEvent evt;
          evt.from = e;
          evt.to = nearest_target;
          evt.type = DamageType::PHYSICAL;
          evt.amount = 1;

          auto& evts = get_first_component<SINGLE_Events>(r);
          evts.dispatcher->trigger(evt);
          evts.dispatcher->update();
        }

        //
      }
    }

    // set the transform position (with offset due to punching)
    const auto pos = body_c.body->GetPosition();

    const float scale = spike(offset_c.t);
    auto offset = scale * glm::vec3{ offset_c.max_offset.x, offset_c.max_offset.y, 0.0f };

    transform_c.position = glm::vec3{ pos.x, pos.y, 0.0f } + offset;
    transform_c.rotation_radians.z = body_c.body->GetAngle();
  }

  // reduce offset over time
  //
  for (const auto& [e, offset_c] : r.view<TransformOffset>().each()) {
    const float speed = 5.0f;
    offset_c.t += dt * speed;
    offset_c.t = glm::clamp(offset_c.t, 0.0f, 1.0f);

    // const auto round_to_zero = [](float val, float amount) {
    //   if (val != 0.0f) {
    //     auto sign = glm::sign(val);
    //     val -= sign * amount;
    //     if (val > 0 && sign < 0)
    //       return 0.0f; // crossed threshold
    //     if (val < 0 && sign > 0)
    //       return 0.0f; // crossed threshold
    //   }
    //   return val;
    // };
    // offset_c.offset.x = round_to_zero(offset_c.offset.x, dt * 100.0f);
    // offset_c.offset.y = round_to_zero(offset_c.offset.y, dt * 100.0f);
    // if (offset_c.offset == glm::vec2(0.0f, 0.0f))
    //   r.remove<TransformOffset>(e);
  }

  /*
  // Debug: visulize velocity...
  static EntityPool pool;
  const auto& physics_view = r.view<const PhysicsBodyComponent, const ActionKey>();
  pool.update(r, physics_view.size_hint() * 3); // each has x and y comp and ddebug nearest

  for (int i = 0; const auto& [e, physics_c, action_key] : physics_view.each()) {
    const auto& vel = physics_c.body->GetLinearVelocity();
    const auto& pos = physics_c.body->GetPosition();

    const auto nearest_target = query_aabb_get_nearest_target(r, e);
    const auto pa = get_position(r, e);
    const auto pb = get_position(r, nearest_target);
    const auto d = pb - pa;

    const auto x_e = pool.instances[i + 0];
    const auto y_e = pool.instances[i + 1];
    const auto dir_e = pool.instances[i + 2];
    i += 3;

    const auto size_x = glm::vec2{ vel.x, 1.0 };
    const auto size_y = glm::vec2{ 1.0, vel.y };
    set_position(r, x_e, glm::vec2{ pos.x, pos.y } + 0.5f * size_x);
    set_position(r, y_e, glm::vec2{ pos.x, pos.y } + 0.5f * size_y);
    set_size(r, x_e, size_x);
    set_size(r, y_e, size_y);

    const auto line = generate_line(pb, pa, 2);
    set_transform_with_line(r.get<TransformComponent>(dir_e), line);

    //
  }
  */
}

} // namespace game2d