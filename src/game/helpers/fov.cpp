#include "fov.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/events/helpers/mouse.hpp" // shouldnt be here
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
update_tile_fov_system(entt::registry& r)
{
  const auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  // visible no hp tiles
  {
    auto visible_no_hp_view = r.view<const VisibleComponent, SpriteColourComponent, const EntityTypeComponent>();
    visible_no_hp_view.each(
      [&r, &colours](const VisibleComponent& v, SpriteColourComponent& scc, const EntityTypeComponent& et) {
        //
        scc.colour = colours.lin_white;
      });
  }

  // visible hp tiles
  auto visible_hp_view =
    r.view<const VisibleComponent, SpriteColourComponent, const HealthComponent, const EntityTypeComponent>();
  {
    visible_hp_view.each([&colours](auto e,
                                    const VisibleComponent& v,
                                    SpriteColourComponent& scc,
                                    const HealthComponent& hp,
                                    const EntityTypeComponent& et) {
      //
      scc.colour = colours.lin_cyan;
    });
  }

  //  not visible tiles
  {
    auto not_visible_view = r.view<const NotVisibleComponent, SpriteColourComponent>();
    not_visible_view.each([&colours](const NotVisibleComponent& v, SpriteColourComponent& scc) {
      //
      scc.colour = colours.lin_black;
    });
  }

  // was visible tiles (no hp)
  {
    auto was_visible_view = r.view<const NotVisibleButPreviouslySeenComponent, SpriteColourComponent>();
    was_visible_view.each([&colours](const NotVisibleButPreviouslySeenComponent& v, SpriteColourComponent& scc) {
      //
      scc.colour = colours.lin_red;
    });
  }

  // was  visible tiles (hp)
  {
    auto was_visible_with_hp_view = r.view<const NotVisibleButPreviouslySeenComponent,
                                           SpriteColourComponent,
                                           const HealthComponent,
                                           const EntityTypeComponent>();
    was_visible_with_hp_view.each([&colours](const NotVisibleButPreviouslySeenComponent& v,
                                             SpriteColourComponent& scc,
                                             const HealthComponent& hp,
                                             const EntityTypeComponent& et) {
      //
      scc.colour = colours.lin_orange;
    });
  }

  // HACK
  if (get_mouse_lmb_held()) {
    for (const auto& visible_entity_with_hp : visible_hp_view)
      r.destroy(visible_entity_with_hp);
  }
};

} // namespace game2d