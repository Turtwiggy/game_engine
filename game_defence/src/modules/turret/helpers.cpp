// #include "helpers.hpp"

// #include <algorithm>
// #include <optional>

// namespace game2d {

// ClosestInfo
// get_closest(const entt::registry& r, const entt::entity& e, const TransformComponent& t, const EntityType& type)
// {
//   ClosestInfo info;

//   std::optional<int> idx_x;
//   std::optional<int> idx_y;

//   // a.x_tl < b.x_tl;
//   auto it_x = std::find(physics.sorted_x.begin(), physics.sorted_x.end(), e);
//   if (it_x != physics.sorted_x.end())
//     idx_x = it_x - physics.sorted_x.begin();

//   // a.y_tl < b.y_tl;
//   auto it_y = std::find(physics.sorted_y.begin(), physics.sorted_y.end(), e);
//   if (it_y != physics.sorted_y.end())
//     idx_y = it_y - physics.sorted_y.begin();

//   if (!idx_x.has_value())
//     return info; // this turret missing from the sorted entity list?
//   if (!idx_y.has_value())
//     return info; // this turret missing from the sorted entity list?

//   auto evaluate_closest = [&r, &t](const std::vector<entt::entity>& sorted, EntityType type, int i) -> ClosestInfo {
//     ClosestInfo oinfo;
//     auto other_entity = sorted[i];
//     auto other_type = r.get<EntityTypeComponent>(other_entity);

//     // check type is of interest
//     if (other_type.type != type)
//       return oinfo; // early exit

//     // calculate distance
//     const auto& other_pos = r.get<TransformComponent>(other_entity);
//     auto d = t.position - other_pos.position;
//     int d2 = d.x * d.x + d.y * d.y;

//     // update info
//     oinfo.e = other_entity;
//     oinfo.distance2 = d2;
//     return oinfo;
//   };

//   // check left...
//   for (int i = idx_x.value() - 1; i >= 0; i--) {
//     const auto oinfo = evaluate_closest(physics.sorted_x, type, i);
//     if (oinfo.distance2 < info.distance2)
//       info = oinfo;
//   }

//   // check right...
//   for (int i = idx_x.value() + 1; i < physics.sorted_x.size(); i++) {
//     const auto oinfo = evaluate_closest(physics.sorted_x, type, i);
//     if (oinfo.distance2 < info.distance2)
//       info = oinfo;
//   }

//   // check up... (y gets less)
//   for (int i = idx_y.value() - 1; i >= 0; i--) {
//     const auto oinfo = evaluate_closest(physics.sorted_y, type, i);
//     if (oinfo.distance2 < info.distance2)
//       info = oinfo;
//   }

//   // check down... (y gets greater)
//   for (int i = idx_y.value() + 1; i < physics.sorted_y.size(); i++) {
//     const auto oinfo = evaluate_closest(physics.sorted_y, type, i);
//     if (oinfo.distance2 < info.distance2)
//       info = oinfo;
//   }

//   return info;
// };

// } // namespace game2d