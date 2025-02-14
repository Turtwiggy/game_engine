#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct HardpointData
{
  std::string key;
  int mount_size = 1;
  float arc_mid = 55;
  float arc = 225;
  float x_rel_tl = 22.5;
  float y_rel_tl = 4.5;

  friend void to_json(nlohmann ::json& j, const HardpointData& val)
  {
    j["s_key"] = val.key;
    j["n_mount_size"] = val.mount_size;
    j["n_arc_mid"] = val.arc_mid;
    j["n_arc"] = val.arc;
    j["n_x_rel_tl"] = val.x_rel_tl;
    j["n_y_rel_tl"] = val.y_rel_tl;
  }
  friend void from_json(const nlohmann ::json& j, HardpointData& val)
  {
    j.at("s_key").get_to(val.key);
    j.at("n_mount_size").get_to(val.mount_size);
    j.at("n_arc_mid").get_to(val.arc_mid);
    j.at("n_arc").get_to(val.arc);
    j.at("n_x_rel_tl").get_to(val.x_rel_tl);
    j.at("n_y_rel_tl").get_to(val.y_rel_tl);
  }
};

struct ShipHullData
{
  std::string name;
  std::string desc;
  int height = 1;
  int width = 1;
  float forward = 0; // 0 = along x axis. +90 = 'up', +270 = 'down'
  std::vector<HardpointData> hardpoints;

  friend void to_json(nlohmann ::json& j, const ShipHullData& val)
  {
    j["s_name"] = val.name;
    j["s_description"] = val.desc;
    j["n_height_px"] = val.height;
    j["n_width_px"] = val.width;
    j["n_forward_dir_angle"] = val.forward;
    j["a_hardpoints"] = val.hardpoints;
  }
  friend void from_json(const nlohmann ::json& j, ShipHullData& val)
  {
    j.at("s_name").get_to(val.name);
    j.at("s_description").get_to(val.desc);
    j.at("n_height_px").get_to(val.height);
    j.at("n_width_px").get_to(val.width);
    j.at("n_forward_dir_angle").get_to(val.forward);
    j.at("a_hardpoints").get_to(val.hardpoints);
  }
};

struct HardpointComponent
{
  HardpointData data;

  // Calculated info...
  glm::vec2 dir_arc_left{ 0, 0 };
  glm::vec2 dir_arc_center{ 0, 0 };
  glm::vec2 dir_arc_right{ 0, 0 };
};

struct ShipHullComponent
{
  bool placeholder = true;
  // ShipHullData data;
};

// data loaded from assets/raws/hulls...
//
struct SINGLE_Hulls
{
  std::vector<ShipHullData> hulls;
};

} // namespace game2d