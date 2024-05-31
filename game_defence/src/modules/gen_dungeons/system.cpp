#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_spaceship_designer/components.hpp"
#include "renderer/transform.hpp"

#include "imgui.h"

#include <SDL_scancode.h>
#include <iostream>

namespace game2d {

void
update_gen_dungeons_system(entt::registry& r)
{
  ImGui::Begin("Debug__Tunnels");
  for (const auto& [e, t_c] : r.view<Tunnel>().each()) {
    ImGui::Separator();

    ImGui::Text("TunnelLine0");
    for (const auto& t : t_c.line_0)
      ImGui::Text("%i %i", t.first, t.second);

    ImGui::Text("TunnelLine1");
    for (const auto& t : t_c.line_1)
      ImGui::Text("%i %i", t.first, t.second);
  }
  ImGui::End();

  // HACK: force regenerate dungeon
  // auto& input = get_first_component<SINGLETON_InputComponent>(r);
  // if (get_key_down(input, SDL_SCANCODE_R))
  //   r.emplace<RequestGenerateDungeonComponent>(r.create());

  const auto& requests = r.view<RequestGenerateDungeonComponent>();
  if (requests.size() == 0)
    return;
  r.destroy(requests.begin(), requests.end()); // process request

  move_to_scene_start(r, Scene::dungeon_designer); // clear old map

  const auto& data_e = get_first<SINGLE_DuckgameToDungeon>(r);
  if (data_e == entt::null) {
    std::cout << "not generating dungeon. SINGLE_DuckgameToDungeon not set." << std::endl;
    return;
  }
  const auto& data = r.get<SINGLE_DuckgameToDungeon>(data_e);
  std::cout << "generating dungeon... you hit a patrol! strength: " << data.patrol_that_you_hit.strength << std::endl;

  // todo: make strength impact the number of things spawned
  const int strength = data.patrol_that_you_hit.strength;
  const auto& map = get_first_component<MapComponent>(r);

  static int seed = 0;
  seed++; // Increase seed everytime a map is generated
  engine::RandomState rnd(seed);

  DungeonGenerationCriteria dungeon_parameters;
  dungeon_parameters.max_rooms = 50;
  dungeon_parameters.room_size_min = glm::max(dungeon_parameters.room_size_min, 0);
  dungeon_parameters.room_size_max = glm::min(dungeon_parameters.room_size_max, map.xmax);

  const DungeonGenerationResults result = generate_rooms(r, dungeon_parameters, rnd);

  // todo: get tunnel lines & apply clipping to walls
  instantiate_tunnels(r, result);
  instantiate_walls(r, result);

  // Steps after initial initialization...
  set_generated_entity_positions(r, result, rnd);
  set_player_positions(r, result, rnd);

  // set exit door position
  // bug: exit placed on generated entities. not so bad?
  // Room& room = d.rooms[d.rooms.size() - 1];
  // glm::ivec2 middle = room_center(room);
  // create_dungeon_entity(editor, game, EntityType::tile_type_exit, middle);

  // Set the cost for all of the tiles
  // for (int i = 0; i < dungeon_result.wall_or_floors.size(); i++) {
  //   // const auto grid_xy = engine::grid::index_to_grid_position(i, map.xmax, map.ymax);
  //   const auto empty_e = create_gameplay(r, EntityType::empty_no_transform);
  //   map.map[i].push_back(empty_e);
  //   auto& pathfinding_c = r.emplace<PathfindComponent>(empty_e);
  //   if (dungeon_result.wall_or_floors[i] == 1)
  //     pathfinding_c.cost = -1; // impassable wall
  //   else
  //     pathfinding_c.cost = 0; // floor
  // }

  std::cout << "dungeon generated" << std::endl;
}

} // namespace game2d