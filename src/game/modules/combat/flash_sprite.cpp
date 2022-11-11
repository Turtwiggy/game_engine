#include "flash_sprite.hpp"

#include "components.hpp"
#include "game/components/actors.hpp"
#include "modules/sprites/components.hpp"

namespace game2d {

void
update_flash_sprite_system(GameEditor& editor, Game& game, uint64_t milliseconds_dt)
{
  const auto& colours = editor.colours;
  auto& r = game.state;

  for (const auto [entity, sc, scc, flash, etc] :
       r.view<SpriteComponent, SpriteColourComponent, FlashSpriteComponent, EntityTypeComponent>().each()) {
    if (!flash.started) {
      flash.started = true;
      scc.colour = colours.lin_hit;
    } else {
      flash.milliseconds_left -= milliseconds_dt;

      if (flash.milliseconds_left <= 0) {
        SpriteColourComponent spr = create_colour(editor, etc.type);
        scc.colour = spr.colour;
        r.remove<FlashSpriteComponent>(entity);
      }
    }
  }
};

} // namespace game2d