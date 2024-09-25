#include "system.hpp"

#include "components.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "imgui.h"
#include "modules/actor_enemy/components.hpp"
#include "modules/animations/wiggle/components.hpp"
#include "modules/map/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_entered_new_room/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include "fmt/core.h"

namespace game2d {
using namespace std::literals;

void
create_imgui_quip(entt::registry& r, const entt::entity actor, const RequestQuip& req, const std::string& quip)
{
  // forward the quip text on to the worldspace text system
  const auto e = create_transform(r, "QUIP");
  r.get<TagComponent>(e).tag = "empty_with_transform:Quip:BEGIN_ENCOUNTER";
  set_position(r, e, get_position(r, actor));
  set_size(r, e, { 0, 0 });

  auto& ui = r.emplace<WorldspaceTextComponent>(e);
  ui.flags = ImGuiWindowFlags_NoDecoration;
  ui.flags |= ImGuiWindowFlags_NoBackground;

  ui.layout = [quip]() {
    //
    // ui.text = quip;
    // ui.offset.y = (-get_size(r, actor).y) / 2.0f;
    // ui.split_text_into_lines = true;
    // ui.line_length = 20;
    ImGui::Text("%s", quip.c_str());
  };

  WiggleUpAndDown wig;
  wig.base_position = get_position(r, actor);
  wig.amplitude = 1.0;
  r.emplace<WiggleUpAndDown>(e, wig);
  r.emplace<EntityTimedLifecycle>(e, static_cast<int>(req.seconds_to_quip * 1000));
};

void
update_quips_system(entt::registry& r)
{
  if (get_first<SINGLE_QuipsComponent>(r) == entt::null)
    return;
  auto& quips = get_first_component<SINGLE_QuipsComponent>(r);

  static engine::RandomState rnd;

  //
  // Process ENTER_ROOM quip type
  // Damage request for QUIPs are in the resolve_collisions()
  //
  const auto map_e = get_first<MapComponent>(r);
  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (map_e != entt::null && dungeon_e != entt::null) {
    const auto& map = get_first_component<MapComponent>(r);
    const auto& dungeon = get_first_component<DungeonGenerationResults>(r);
    {
      const auto& reqs = r.view<PlayerEnteredNewRoom>();
      for (const auto& [req_e, req_c] : reqs.each()) {
        const auto req_room = req_c.room_e;

        std::vector<entt::entity> enemies_in_newly_entered_room;

        // check the number of enemies in the room.
        const auto& enemies_view = r.view<EnemyComponent>();
        for (const auto& [enemy_e, enemy_c] : enemies_view.each()) {
          const auto enemy_pos = get_position(r, enemy_e);
          const auto enemy_gridpos = engine::grid::worldspace_to_grid_space(enemy_pos, map.tilesize);

          const auto rooms = inside_room(r, enemy_gridpos);

          const auto in_room = rooms.size() > 0;
          if (!in_room)
            continue;

          if (req_room != rooms[0])
            continue; // enemy not in the room we just entered

          enemies_in_newly_entered_room.push_back(enemy_e);
        }

        // must be enemies
        if (enemies_in_newly_entered_room.size() == 0)
          continue;

        // choose a random enemy to say the quip
        const int enemy_to_quip_idx = engine::rand_det_s(rnd.rng, 0, (int)enemies_in_newly_entered_room.size());
        const entt::entity enemy_to_quip = enemies_in_newly_entered_room[enemy_to_quip_idx];

        // roll a dice to say a quip.
        const bool should_quip = engine::rand_01(rnd.rng) < 0.3f;
        if (!should_quip)
          return;

        // request quip
        RequestQuip quip_req;
        quip_req.type = QuipType::ENTER_ROOM;
        quip_req.quipp_e = enemy_to_quip;
        create_empty<RequestQuip>(r, quip_req);
      }

      // done all requests...
      r.destroy(reqs.begin(), reqs.end());
    }
  }

  //
  // Quip system
  //

  const auto& quip_req = r.view<RequestQuip>();
  for (const auto& [req_e, quip_c] : quip_req.each()) {
    const auto& e_to_quip = quip_c.quipp_e;

    const auto recycle_quips = [](auto& unused, const auto& available) {
      if (unused.size() == 0)
        unused = std::vector<std::string>(available.begin(), available.end());
    };
    recycle_quips(quips.quips_unused, quips.quips);
    recycle_quips(quips.quips_hit_unused, quips.quips_hit);
    recycle_quips(quips.quips_encounter_unused, quips.quips_encounter);

    const auto get_quip = [](auto& quips) -> std::string {
      // pull the quip from the unused pile
      const int rnd_idx = int(engine::rand_det_s(rnd.rng, 0, int(quips.size())));
      const auto quip = quips[rnd_idx];
      quips.erase(quips.begin() + rnd_idx);
      return quip;
    };

    std::string quip = ":O";

    if (quip_c.type == QuipType::BEGIN_ENCOUNTER)
      quip = get_quip(quips.quips_encounter_unused);
    else if (quip_c.type == QuipType::ENTER_ROOM)
      quip = get_quip(quips.quips_unused);
    else if (quip_c.type == QuipType::TOOK_DAMAGE)
      quip = get_quip(quips.quips_hit_unused);
    else
      fmt::println("WARNING: not implemented quip scenario");

    create_imgui_quip(r, e_to_quip, quip_c, quip);

    // destroy quip reqs...
    r.destroy(quip_req.begin(), quip_req.end());
  }
};

} // namespace game2d