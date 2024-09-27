#include "breach_charge_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/map/components.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

/*
void
create_o2_particle(entt::registry& r, entt::entity dead_e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& edge = r.get<Edge>(dead_e);
  const auto pos_a_worldspace = engine::grid::grid_space_to_world_space_center(edge.gp_a, map.tilesize);
  const auto pos_b_worldspace = engine::grid::grid_space_to_world_space_center(edge.gp_b, map.tilesize);

  const auto rooms_a = inside_room(r, edge.gp_a);
  const auto rooms_b = inside_room(r, edge.gp_b);
  const auto in_room = rooms_a.size() > 0 || rooms_b.size() > 0;

  // add a breachedroom component...
  // for (const auto room_e : rooms_a)
  //   r.emplace_or_replace<BreachedRoom>(room_e);
  // for (const auto room_e : rooms_b)
  //   r.emplace_or_replace<BreachedRoom>(room_e);

  // Point the particles outside the room
  glm::vec2 dir{ 0.0f, 0.0f };
  if (in_room)
    dir = engine::normalize_safe(pos_b_worldspace - pos_a_worldspace);
  else
    dir = -engine::normalize_safe(pos_b_worldspace - pos_a_worldspace);

  const auto halfway_pos = (pos_a_worldspace + pos_b_worldspace) * 0.5f;

  // a particle_parent_e is what the particle emitter follows
  const auto particle_parent_e = create_transform(r);
  r.get<TagComponent>(particle_parent_e).tag = "particle_emitter_parent";
  set_position(r, particle_parent_e, halfway_pos);
  set_size(r, particle_parent_e, { 0, 0 }); // no size for particle emitter

  // add_particles()
  DataParticleEmitter desc;
  desc.parent = particle_parent_e;
  desc.velocity = dir * 20.0f;
  desc.start_size = 10;
  desc.end_size = 2;
  desc.colour = engine::SRGBColour{ 0.9f, 0.9f, 0.9f, 0.9f };
  auto particle_e = Factory_DataParticleEmitter::create(r, desc);

  fmt::println("spawning particle emitter due to blown up edge at {} {}", halfway_pos.x, halfway_pos.y);
}
*/

void
add_bomb_callback(entt::registry& r, const entt::entity add_to_e)
{
  OnDeathCallback callback;
  callback.callback = [](entt::registry& r, const entt::entity e) {
    const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

    // create a boom effect
    const glm::vec2 pos = get_position(r, e);
    create_empty<RequestToSpawnParticles>(r, RequestToSpawnParticles{ pos });

    // open up any edges surrounding the boom
    // Did I accidentally create bomberman?
    const auto& physics = get_first_component<SINGLE_Physics>(r);
    auto& map = get_first_component<MapComponent>(r);
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map.tilesize);
    // fmt::println("go boom at {}, {}. gp: {}, {}", pos.x, pos.y, gp.x, gp.y);

    // Have to use gridpos not index, because gridpos {-1, 12} is valid for a bomb to be placed
    // but if that was convertd to an index it would wrap round the value and be too large.
    const auto neighbours = engine::grid::get_neighbour_gridpos(gp, map.xmax, map.ymax);
    for (const auto& [dir, n_gp] : neighbours) {
      const auto edge_e = edge_between_gps(r, gp, n_gp);
      if (edge_e == entt::null)
        continue; // no edge
      const Edge& edge = r.get<Edge>(edge_e);
      fmt::println("removing edge..");

      // destroy body immediately
      auto& physics_b = r.get<PhysicsBodyComponent>(edge_e).body;
      physics.world->DestroyBody(physics_b);
      r.remove<PhysicsBodyComponent>(edge_e);

      // destroy edge
      dead.dead.emplace(edge_e);
    }

    // create a particle spawner...
    // for (const auto& dead_e : dead.dead)
    //   if (auto* edge_c = r.try_get<Edge>(dead_e))
    //     create_o2_particle(r, dead_e);

    // request some audio
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "BOMB_BLOWUP_01" });
  };
  r.emplace<OnDeathCallback>(add_to_e, callback);
};

bool
debug_spawn_bombs(entt::registry& r, const glm::ivec2& mouse_pos_on_grid)
{
  // debug spawn bombs immediately
  bool imediately_place_bomb = false;

#if defined(_DEBUG)
  imediately_place_bomb = false;
#endif

  // if (imediately_place_bomb && get_mouse_lmb_press()) {
  //   // fmt::println("immediately placing bomb...");
  //   // DataBreachCharge desc;
  //   // desc.pos = glm::vec2(mouse_pos_on_grid);
  //   // const auto charge_e = Factory_DataBreachCharge::create(r, desc);
  // }

  return imediately_place_bomb;
};

entt::entity
bomb_equipped_in_inventory(entt::registry& r)
{
  const auto& view = r.view<const PlayerComponent, DefaultBody>();

  for (const auto& [e, player_c, equipment_c] : view.each()) {
    for (const auto& body_e : equipment_c.body) {
      const auto& slot_c = r.get<InventorySlotComponent>(body_e);
      if (slot_c.item_e == entt::null)
        continue; // no item;
      const auto& item_c = r.get<UI_ItemComponent>(slot_c.item_e);
      const auto& tag_c = r.get<TagComponent>(slot_c.item_e);

      if (tag_c.tag.find("breach_charge") != std::string::npos)
        return slot_c.item_e;
    }
  }

  return entt::null;
};

void
DrawZeldaCursor(const ImVec2& pos, const ImVec2& window_size, float radius, float thickness, float angle, ImU32 color)
{
  const ImVec2 center = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
  const float max_angle = 2.0f * engine::PI;

  if (angle > max_angle)
    angle = max_angle;
  const float start_angle = -engine::PI / 2; // Starting at the top (12 o'clock position)
  const float end_angle = start_angle + (angle / max_angle) * 2.0f * engine::PI;

  // ImDrawList* draw_list = ImGui::GetWindowDrawList();
  // Use the foreground draw list so it's drawn on top of everything
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();

  ImVec2 window_center = { center.x + window_size.x / 2.0f, center.y + window_size.y / 2.0f };
  // window_center.y += radius / 2.0f; // move circle to center

  draw_list->PathArcTo(window_center, radius, start_angle, end_angle);
  draw_list->PathStroke(color, false, thickness);
};

void
DrawZeldaCursorWindow(entt::registry& r, const glm::ivec2& mouse_pos, const float angle)
{
  const float radius = 10.0f;
  const float thickness = 6.0f;
  const ImU32 color = IM_COL32(0, 255, 0, 255);

  const auto t_pos = glm::ivec2(mouse_pos.x, mouse_pos.y);
  const auto worldspace = position_in_worldspace(r, t_pos);
  const ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };
  const ImVec2 size = ImVec2(200, 200);
  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos({ pos.x - size.x / 2.0f, pos.y - size.y / 2.0f });

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Disable padding
  ImGui::Begin("cursor-timer", NULL, flags);
  ImGui::PopStyleVar();

  DrawZeldaCursor(pos, size, radius, thickness, angle, color);

  ImGui::End();
};

} // namespace game2d