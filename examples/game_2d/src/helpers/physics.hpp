#pragma once

// c++ lib headers
#include <map>
#include <vector>

namespace game2d {

struct AABB
{
  int x_tl;
  int y_tl;
  int w;
  int h;
};

struct PhysicsObject
{
  int ent_id;
  AABB aabb;
};

struct Collision2D
{
  int ent_id_0;
  int ent_id_1;
};

//     std::vector<std::reference_wrapper<GameObject2D>> collidable;
//     collidable.insert(collidable.end(), gs.entities_enemies.begin(), gs.entities_enemies.end());
//     collidable.insert(collidable.end(), gs.entities_bullets.begin(), gs.entities_bullets.end());
//     collidable.insert(collidable.end(), gs.entities_player.begin(), gs.entities_player.end());
//     collidable.insert(collidable.end(), gs.entities_trees.begin(), gs.entities_trees.end());
//     collidable.push_back(gs.weapon_shovel);
//     collidable.push_back(pushable);
//     // generate filtered broadphase collisions.
//     std::map<uint64_t, Collision2D> filtered_collisions;
//     generate_filtered_broadphase_collisions(active_collidable, filtered_collisions);
//     // Add collision to events
//     for (auto& c : filtered_collisions) {
//       uint32_t id_0 = c.second.ent_id_0;
//       uint32_t id_1 = c.second.ent_id_1;
//       // Find the objs in the read-only list
//       auto& obj_0_it = std::find_if(
//         collidable.begin(), collidable.end(), [&id_0](const auto& obj) { return obj.get().id == id_0; });
//       auto& obj_1_it = std::find_if(
//         collidable.begin(), collidable.end(), [&id_1](const auto& obj) { return obj.get().id == id_1; });
//       if (obj_0_it == collidable.end() || obj_1_it == collidable.end()) {
//         std::cerr << "Collision entity not in entity list" << std::endl;
//         continue;
//       }
//       CollisionEvent eve(obj_0_it->get(), obj_1_it->get());
//       gs.collision_events.push_back(eve);
//     }

// broadphase: detect collisions that can actually happen and discard collisions which can't.
// sort and prune algorithm. note: suffers from large worlds with inactive objects.
// this issue can be solved by using multiple smaller SAP's which form a grid.
// note: i've adjusted this algortihm to do 2-axis SAP.
void
generate_filtered_broadphase_collisions(const std::vector<std::reference_wrapper<PhysicsObject>>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collisions);

} // namespace game2d
