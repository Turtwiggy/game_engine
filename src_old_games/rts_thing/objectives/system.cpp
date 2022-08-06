#include "system.hpp"

#include "components.hpp"

// my libs
#include "game/components/selectable.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

void
game2d::update_objectives_system(entt::registry& registry)
{
  const auto& p = registry.ctx().at<SINGLETON_PhysicsComponent>();
  const float OBJECTIVE_FADE = 0.5f;

  {
    const auto& view = registry.view<const ObjectiveComponent, const HighlightComponent>();
    view.each([&p, &registry, &OBJECTIVE_FADE](auto entity, const auto& obj, const auto& highlight) {
      //
      // Now check physics systems for objective-unit collisions

      entt::entity e0_objective = entt::null;
      entt::entity e1_unit = entt::null;

      for (const auto& coll : p.collision_stay) {

        const auto e0_id = static_cast<entt::entity>(coll.ent_id_0);
        const auto& e0_layer = registry.get<PhysicsActorComponent>(e0_id);
        const auto e0_layer_id = e0_layer.layer_id;

        const auto e1_id = static_cast<entt::entity>(coll.ent_id_1);
        const auto& e1_layer = registry.get<PhysicsActorComponent>(e1_id);
        const auto e1_layer_id = e1_layer.layer_id;

        // check that this objective is the entity collision
        if (entity != e0_id && entity != e1_id)
          continue;

        // Collisions are bi-directional, but only one collision exists
        if (e0_layer_id == GameCollisionLayer::ACTOR_UNIT_GROUP && e1_layer_id == GameCollisionLayer::ACTOR_OBJECTIVE) {
          e0_objective = e1_id;
          e1_unit = e0_id;
        }
        if (e0_layer_id == GameCollisionLayer::ACTOR_OBJECTIVE && e1_layer_id == GameCollisionLayer::ACTOR_UNIT_GROUP) {
          e0_objective = e0_id;
          e1_unit = e1_id;
        }
      }

      // if (e0_objective != entt::null && e1_unit != entt::null) {
      //   if (registry.all_of<HighlightComponent>(e1_unit)) {
      //     // change the colour of this objective to the colour of the unit occupying it
      //     const auto unit_colour = registry.get<HighlightComponent>(e1_unit).start_colour;
      //     colour.colour = unit_colour * OBJECTIVE_FADE;
      //   } else {
      //     colour.colour = highlight.start_colour;
      //   }
      // }
    });
  }
};