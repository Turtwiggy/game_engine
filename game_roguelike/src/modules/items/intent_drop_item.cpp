#include "intent_drop_item.hpp"

#include "components.hpp"
#include "components/actors.hpp"

namespace game2d {

void
update_intent_drop_item_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  // // WantsToDrop
  // const auto& drop_view = r.view<const TransformComponent, WantsToDrop>();
  // drop_view.each([&r, &editor, &game](auto entity, const auto& actor, auto& intent) {
  //   //
  //   for (const entt::entity& item : intent.items) {
  //     const EntityTypeComponent type = r.get<EntityTypeComponent>(item);

  //     // note, the two below are the opposite of the
  //     // clean() function which removes renderable and physics

  //     // // add physics
  //     // create_gameplay_existing_entity(editor, game, item, type.type);

  //     // // make renderable
  //     // create_renderable(editor, r, item, type.type);

  //     // auto& transform = r.get<TransformComponent>(item);
  //     // transform.position.x = actor.position.x;
  //     // transform.position.y = actor.position.y;

  //     // r.remove<InBackpackComponent>(item);
  //   }
  //   // r.remove<WantsToDrop>(entity);
  // });
};

} // namespace game2d
