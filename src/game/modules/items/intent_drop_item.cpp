#include "intent_drop_item.hpp"

namespace game2d {

void
update_intent_drop_item(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  // WantsToDrop
  // const auto& drop_view = r.view<const TransformComponent, WantsToDrop>();
  // drop_view.each([&r](auto entity, const auto& actor, auto& intent) {
  //   //
  //   for (const auto& item : intent.items) {
  //     create_renderable(editor, item, EntityType::potion);
  //     auto& transform = r.get<TransformComponent>(item);
  //     transform.position.x = actor.position.x;
  //     transform.position.y = actor.position.y;
  //     r.remove<InBackpackComponent>(item);
  //   }
  //   r.remove<WantsToDrop>(entity);
  // });
};

} // namespace game2d
