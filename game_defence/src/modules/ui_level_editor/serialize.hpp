#pragma once

// other lib headers
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

// std libs
#include <string>

namespace game2d {
using namespace nlohmann;

class NJSONOutputArchive
{
public:
  NJSONOutputArchive() = default;

  inline void operator()(std::underlying_type_t<entt::entity> size)
  {
    if (!current.empty())
      root.push_back(current);
    current = json::array();
    current.push_back(size); // first element of each array keeps the amount of elements.
  }

  // persist entity ids
  inline void operator()(entt::entity entity)
  {
    current.push_back((uint32_t)entity);
    //
  };

  // persist components
  template<typename T>
  void operator()(const T& t)
  {
    json json = t;
    current.push_back(json);
  }

  inline void close()
  {
    if (!current.empty())
      root.push_back(current);
  }

  inline const std::string as_string() { return root.dump(); }
  inline const std::vector<uint8_t> as_bson() { json::to_bson(root); }

private:
  json root = json::array();
  json current;
};

class NJSONInputArchive
{
private:
  json root;
  json current;
  int root_idx = -1;
  int current_idx = 0;

public:
  NJSONInputArchive(const std::string& json_string)
  { //
    root = nlohmann::json::parse(json_string);
  }

  inline void next_root()
  {
    root_idx++;
    if (root_idx >= root.size()) {
      // ERROR
      return;
    }
    current = root[root_idx];
    current_idx = 0;
  }

  inline void operator()(std::underlying_type_t<entt::entity>& s)
  {
    next_root();
    int size = current[0].get<int>();
    current_idx++;
    s = (std::underlying_type_t<entt::entity>)size; // pass amount to entt
  }

  inline void operator()(entt::entity& entity)
  {
    uint32_t ent = current[current_idx].get<uint32_t>();
    entity = entt::entity(ent);
    current_idx++;
  }

  template<typename T>
  void operator()(T& t)
  {
    json component_data = current[current_idx * 2];

    auto comp = component_data.get<T>();
    t = comp;

    uint32_t _ent = current[current_idx * 2 - 1];
    // ent = entt::entity(_ent); // last element is the entity-id
    current_idx++;
  }
};

} // namespace game2d