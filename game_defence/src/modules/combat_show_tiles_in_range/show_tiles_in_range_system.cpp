#include "modules/combat_show_tiles_in_range/show_tiles_in_range_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"

namespace game2d {

enum class RangeType
{
  knife = 0,
  shotgun,

  count,
};

//  X
// XOX
//  X
std::vector<glm::ivec2>
get_tiles_for_knife(entt::registry& r, const MapComponent& map_c, const glm::ivec2& pos)
{
  return generate_accessible_areas(r, map_c, pos, 1);
};

//  YY
// OXY
//  YY
std::vector<glm::ivec2>
get_tiles_for_shotgun(entt::registry& r,
                      const MapComponent& map_c,
                      const glm::ivec2& pos,
                      const engine::grid::GridDirection& dir)
{
  // these are your clickable tiles... where you can click to shoot the shotgun
  auto available_tiles = generate_accessible_areas(r, map_c, pos, 2);

  // generate additional tiles around the clickable tile for extra damage for shotgun
  //...

  return available_tiles;
};

template<typename T>
void
hmm() {

  // if first time...
};

static bool show_distance_check = false;
static auto show_range_type = RangeType::knife;
static auto range_types_str = engine::enum_class_to_vec_str<RangeType>();

void
update_show_tiles_in_range_system(entt::registry& r)
{
  const auto& input = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_KP_0))
    show_distance_check = !show_distance_check;
  if (!show_distance_check)
    return;

  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

#if defined(_DEBUG)
  ImGui::Begin("ShowTile Mode");
  WomboComboIn combo_in(range_types_str);
  combo_in.label = "ShowTileMode";
  combo_in.current_index = static_cast<int>(show_range_type);
  WomboComboOut combo_out = draw_wombo_combo(combo_in);
  if (combo_in.current_index != combo_out.selected)
    show_range_type = static_cast<RangeType>(combo_out.selected);
  ImGui::End();
#endif

  static EntityPool pool;

  const auto& view = r.view<PlayerComponent>();
  for (const auto& [e, player_c] : view.each()) {
    const auto grid_pos = get_grid_position(r, e);
    // ImGui::Text("you: %i %i", grid_pos.x, grid_pos.y);

    std::vector<glm::ivec2> tiles;
    if (show_range_type == RangeType::knife)
      tiles = get_tiles_for_knife(r, map_c, grid_pos);
    if (show_range_type == RangeType::shotgun)
      tiles = get_tiles_for_shotgun(r, map_c, grid_pos, engine::grid::GridDirection::east);

    // note: this is bad for multiple players
    pool.update(r, tiles.size());

    for (int i = 0; const auto& tile_gp : tiles) {
      // ImGui::Text("accessible: %i %i", tile_gp.x, tile_gp.y);

      // add a transform, if needed
      const auto debug_e = pool.instances[i];
      if (r.try_get<TransformComponent>(debug_e) == nullptr) {
        TransformComponent t_c;
        t_c.scale = { 8, 8, 8 };
        r.emplace<TransformComponent>(debug_e, t_c);
        r.emplace<SpriteComponent>(debug_e);
        set_sprite(r, debug_e, "EMPTY");
      }

      const auto offset = glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
      const auto anypos = glm::vec2(tile_gp * map_c.tilesize) + offset;
      set_position(r, debug_e, anypos);

      i++;
    }

    break; // only first player
  }
}

} // namespace game2d