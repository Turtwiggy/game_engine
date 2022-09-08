#pragma once

// other lib headers
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

// std libs
#include <string>

namespace game2d {

class NJSONOutputArchive
{
public:
  NJSONOutputArchive();

  // new element for serialization. giving you the amount of elements that is going to be
  // pumped into operator()(entt::entity ent) or operator()(entt::entity ent, const T &t)
  void operator()(std::underlying_type_t<entt::entity> size);

  // persist entity ids
  void operator()(entt::entity entity);

  // persist components
  template<typename T>
  void operator()(entt::entity ent, const T& t)
  {
    current.push_back((uint32_t)ent); // persist the entity id of the following component

    nlohmann::json json = t;
    current.push_back(json);
  }

  void close();

  const std::string as_string();
  const std::vector<uint8_t> as_bson();

private:
  nlohmann::json root;
  nlohmann::json current;
};

class NJSONInputArchive
{
private:
  nlohmann::json root;
  nlohmann::json current;

  int root_idx = -1;
  int current_idx = 0;

public:
  NJSONInputArchive(const std::string& json_string);
  ~NJSONInputArchive();

  void next_root();

  void operator()(std::underlying_type_t<entt::entity>& s);

  void operator()(entt::entity& entity);

  template<typename T>
  void operator()(entt::entity& ent, T& t)
  {
    nlohmann::json component_data = current[current_idx * 2];

    auto comp = component_data.get<T>();
    t = comp;

    uint32_t _ent = current[current_idx * 2 - 1];
    ent = entt::entity(_ent); // last element is the entity-id
    current_idx++;
  }
};

} // namespace game2d