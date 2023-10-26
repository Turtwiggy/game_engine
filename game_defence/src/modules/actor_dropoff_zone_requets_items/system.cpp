#include "system.hpp"

#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/animation/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {
using namespace std::literals;

// this shouldnt be here
static engine::RandomState rnd;

void
update_actor_dropoffzone_request_items(entt::registry& r, uint64_t ms_dt)
{
  const float dt = ms_dt / 1000.0f;
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  const auto& view = r.view<DropoffZoneComponent, SpriteComponent, const AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, zone, sprite, aabb] : view.each()) {

    // change sprite based on customers at zone
    if (zone.cur_customers == 0) {
      set_sprite_custom(r, e, "campfire_empty"s, ri.tex_unit_custom);
      set_sprite_custom(r, zone.sign, "icon_sign_inactive"s, ri.tex_unit_custom);
    } else {
      set_sprite_custom(r, e, "campfire_full"s, ri.tex_unit_custom);
      set_sprite_custom(r, zone.sign, "icon_sign_active"s, ri.tex_unit_custom);
    }

    // create people
    for (int i = zone.cur_customers; i > zone.instantiated_customers.size(); i--) {
      const auto new_customer = create_gameplay(r, EntityType::empty);
      set_sprite_custom(r, new_customer, "customer_0", ri.tex_unit_custom);

      auto& new_transform = r.get<TransformComponent>(new_customer);
      new_transform.position = { aabb.center.x + aabb.size.x / 2, aabb.center.y, 0.0f };
      new_transform.scale = { 32, 32, 1 };
      new_transform.position.x += 50 * i;

      WiggleUpAndDown wiggle;
      wiggle.base_position = { new_transform.position.x, new_transform.position.y };
      r.emplace<WiggleUpAndDown>(new_customer, wiggle);

      r.get<TagComponent>(new_customer).tag = "customer"s;

      zone.instantiated_customers.push_back(new_customer);
    }

    // destroy people
    for (int i = zone.instantiated_customers.size(); i > zone.cur_customers; i--) {
      const auto idx = i - 1;
      auto customer_entity = zone.instantiated_customers[idx];
      r.destroy(customer_entity);
      zone.instantiated_customers.erase(zone.instantiated_customers.begin() + idx);
    }

    if (zone.cur_customers == 0)
      continue; // no customers

    zone.timer -= dt;
    if (zone.timer > 0)
      continue; // on cooldown

    if (zone.requested_items.size() == zone.cur_customers)
      continue; // max requests equal to customers at table

    // if (zone.requested_items.size() >= zone.max_requests)
    //   continue; // not interested

    // Reset timer
    const float random_time = engine::rand_det_s(rnd.rng, zone.min_seconds_between_items, zone.max_seconds_between_items);
    zone.timer = random_time;

    // Add a request to the dropzone
    const float random_idx_f = engine::rand_det_s(rnd.rng, 0, zone.valid_items_to_request.size());
    const int random_idx = static_cast<int>(random_idx_f);
    const int random_item = zone.valid_items_to_request[random_idx];
    zone.requested_items.push_back(random_item);
  }
}

} // namespace game2d