#include "system.hpp"

#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/animation/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
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
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<DropoffZoneComponent, SpriteComponent, const AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, zone, sprite, aabb] : view.each()) {

    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto tex_unit = search_for_texture_unit_by_path(ri, "bargame")->unit;

    // change sprite based on customers at zone
    if (zone.cur_customers == 0) {
      set_sprite_custom(r, e, "campfire_empty"s, tex_unit);
      set_sprite_custom(r, zone.sign, "icon_sign_inactive"s, tex_unit);
    } else {
      set_sprite_custom(r, e, "campfire_full"s, tex_unit);
      set_sprite_custom(r, zone.sign, "icon_sign_active"s, tex_unit);
    }

    // create people
    for (int i = zone.cur_customers; i > zone.instantiated_customers.size(); i--) {
      std::cout << "new i " << i << std::endl;
      const auto new_customer = create_gameplay(r, EntityType::empty);
      set_sprite_custom(r, new_customer, "customer_0", tex_unit);

      auto& new_transform = r.get<TransformComponent>(new_customer);
      new_transform.position = { aabb.center.x, aabb.center.y, 0.0f };
      new_transform.scale = { 32, 32, 1 };

      // top left
      if (i == 1) {
        new_transform.position.x -= aabb.size.x / 4.0f;
        new_transform.position.y -= aabb.size.y / 4.0f;
      }
      // top right
      if (i == 2) {
        new_transform.position.x += aabb.size.x / 4.0f;
        new_transform.position.y -= aabb.size.y / 4.0f;
      }
      // bottom right
      if (i == 3) {
        new_transform.position.x += aabb.size.x / 4.0f;
        new_transform.position.y += aabb.size.y / 4.0f;
      }
      // bottom left
      if (i == 4) {
        new_transform.position.x -= aabb.size.x / 4.0f;
        new_transform.position.y += aabb.size.y / 4.0f;
      }

      WiggleUpAndDown wiggle;
      wiggle.base_position = { new_transform.position.x, new_transform.position.y };
      r.emplace<WiggleUpAndDown>(new_customer, wiggle);

      r.get<TagComponent>(new_customer).tag = "customer"s;

      zone.instantiated_customers.push_back(new_customer);
    }

    // destroy people
    for (auto i = zone.instantiated_customers.size(); i > zone.cur_customers; i--) {
      const auto idx = i - 1;
      auto customer_entity = zone.instantiated_customers[idx];
      dead.dead.emplace(customer_entity);
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