// your header
#include "select_objects.hpp"

// components
#include "game/components/cursor.hpp"
#include "game/components/selectable.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>

// std lib headers
#include <vector>

void
game2d::update_select_objects_system(entt::registry& registry)
{
  // .. get the list of all collisions
  const auto& p = registry.ctx().at<SINGLETON_PhysicsComponent>();
  const auto& input = registry.ctx().at<SINGLETON_InputComponent>();
  {
    const auto& view = registry.view<CursorComponent>();
    view.each([&registry, &p, &input](auto entity, auto& c) {
      //... for each cursor ...

      if (!get_mouse_lmb_press() && !get_mouse_lmb_held()) {
        // user is not holding...
        return;
      }

      const auto& selectable = registry.view<SelectableComponent>();
      selectable.each([&input](auto& selectable) {
        if (get_mouse_lmb_press()) {
          // user clicked... remove all the old selected
          // (i.e. keep persistent until new click)
          selectable.is_selected = false;
        }
      });

      // Now check physics systems for cursor-unit collisions

      for (const auto& coll : p.collision_stay) {

        const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
        const auto& e0_layer = registry.get<PhysicsActorComponent>(e0_id);
        const auto e0_layer_id = e0_layer.layer_id;

        const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
        const auto& e1_layer = registry.get<PhysicsActorComponent>(e1_id);
        const auto e1_layer_id = e1_layer.layer_id;

        entt::entity e0_cursor = entt::null;
        entt::entity e1_unit = entt::null;

        // Collisions are bi-directional, but only one collision exists
        if (e0_layer_id == GameCollisionLayer::ACTOR_UNIT_GROUP && e1_layer_id == GameCollisionLayer::ACTOR_CURSOR) {
          e0_cursor = e1_id;
          e1_unit = e0_id;
        }
        if (e0_layer_id == GameCollisionLayer::ACTOR_CURSOR && e1_layer_id == GameCollisionLayer::ACTOR_UNIT_GROUP) {
          e0_cursor = e0_id;
          e1_unit = e1_id;
        }

        if (e0_cursor != entt::null && e1_unit != entt::null) {
          auto& sc = registry.get<SelectableComponent>(e1_unit);
          sc.is_selected = true;
        }
      }
    });
  }
};
