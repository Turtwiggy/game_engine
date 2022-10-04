#include "fov.hpp"

#include "engine/maths/grid.hpp"
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

  const auto& player_view = r.view<PlayerComponent>();
  if (player_view.size() == 0)
    return; // no player to home in on
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  glm::ivec2 player_grid_pos =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, 16);

  const int GRID_SIZE = 16;

  const auto& view_transforms = r.view<const TransformComponent>();
  view_transforms.each([&r, &player_grid_pos, &player_entity](auto e, const auto& t) {
    // skip self
    if (e == player_entity)
      return;

    const auto grid = engine::grid::world_space_to_grid_space({ t.position.x, t.position.y }, GRID_SIZE);
    const int distance_x = glm::abs(grid.x - player_grid_pos.x);
    const int distance_y = glm::abs(grid.y - player_grid_pos.y);
    const int dst = 4;
    // const int dst = 10; // debug to see whole map
    const bool within_distance = distance_x < dst && distance_y < dst;

    // If it's within the distance, make it visible
    if (within_distance) {
      r.emplace_or_replace<VisibleComponent>(e);
      if (r.try_get<NotVisibleComponent>(e))
        r.remove<NotVisibleComponent>(e);
      if (r.try_get<NotVisibleButPreviouslySeenComponent>(e))
        r.remove<NotVisibleButPreviouslySeenComponent>(e);
    }
    // If it's not within the distance, hide it
    else {

      // if it's already got NotVisibleButPreviouslySeenComponent,
      auto seen_before = r.try_get<NotVisibleButPreviouslySeenComponent>(e);
      if (seen_before)
        return; // change nothing

      // if it was Visible, set as NotVisibleButPreviouslySeenComponent
      if (auto visible = r.try_get<VisibleComponent>(e)) {
        r.remove<VisibleComponent>(e);
        r.emplace_or_replace<NotVisibleButPreviouslySeenComponent>(e);
      }
      // if it was not Visible, just set it back to not visible
      else {
        r.emplace_or_replace<NotVisibleComponent>(e);
      }
    }
  });

  // visible no hp tiles
  {
    auto visible_no_hp_view = r.view<const VisibleComponent, SpriteColourComponent, const EntityTypeComponent>();
    visible_no_hp_view.each(
      [&r, &colours](const VisibleComponent& v, SpriteColourComponent& scc, const EntityTypeComponent& et) {
        //
        scc.colour = colours.lin_red;
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
      scc.colour = colours.lin_orange;
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
      scc.colour = colours.lin_feint_white;
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
      scc.colour = colours.lin_cyan;
    });
  }

  // HACK
  // if (get_mouse_lmb_held()) {
  //   for (const auto& visible_entity_with_hp : visible_hp_view)
  //     r.destroy(visible_entity_with_hp);
  // }
};

} // namespace game2d