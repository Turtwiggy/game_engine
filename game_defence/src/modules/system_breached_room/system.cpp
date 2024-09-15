#include "system.hpp"

#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_breached_room/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_breached_room_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = get_first_component<MapComponent>(r);

  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e == entt::null)
    return;
  const auto dungeon_c = get_first_component<DungeonGenerationResults>(r);

  ImGui::Begin("Debug_BreachedRoom");

  for (const auto& [e, room_c, breached_c] : r.view<Room, BreachedRoom>().each()) {
    //
    ImGui::Text("breached room...");

    //
    // todo: set all the entities in the room as floating towards the breach...
    // todo: set the lights to red
    //
  }

  ImGui::End();

  //
};

} // namespace game2d