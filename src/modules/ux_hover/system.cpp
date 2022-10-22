#include "system.hpp"

#include "components.hpp"
#include "engine/colour.hpp"
#include "modules/cursor/components.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"

namespace game2d {

void
update_ux_hover_system(GameEditor& editor, Game& game)
{
  //
  // work out if an entity is selected
  //
  const auto& physics = game.physics;
  auto& r = game.state;
  const auto& cursor_entity = get_first<FreeCursorComponent>(r);
  if (cursor_entity == entt::null)
    return;
  const auto& cursor = r.get<FreeCursorComponent>(cursor_entity);
  const auto& cursor_collision_area = cursor.backdrop;

  // if (get_mouse_lmb_press())
  //   r.clear<SelectableComponent>();

  // const auto& input = game.input;
  // if (get_mouse_lmb_held()) {

  // for (const auto& coll : physics.collision_stay) {
  //   uint32_t id = static_cast<uint32_t>(cursor_collision_area);
  //   if (coll.ent_id_0 == id) {
  //     auto other = static_cast<entt::entity>(coll.ent_id_1);
  //     r.emplace_or_replace<SelectableComponent>(other);
  //   }
  //   if (coll.ent_id_1 == id) {
  //     auto other = static_cast<entt::entity>(coll.ent_id_0);
  //     r.emplace_or_replace<SelectableComponent>(other);
  //   }
  // }

  //
  // change colours of all selected entities
  //
  const auto& view = r.view<HoverComponent, SpriteColourComponent>();
  for (auto [entity, hover, sprite] : view.each()) {
    auto* selected = r.try_get<SelectableComponent>(entity);
    if (selected)
      sprite.colour = engine::SRGBToLinear(hover.hover_colour);
    else
      sprite.colour = engine::SRGBToLinear(hover.regular_colour);
  }
};

} // namespace game2d