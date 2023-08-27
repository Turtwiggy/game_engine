#include "serialize.hpp"

namespace game2d {

//
// Output Archive
//

NJSONOutputArchive::NJSONOutputArchive()
{
  root = nlohmann::json::array();
};

void
NJSONOutputArchive::operator()(std::underlying_type_t<entt::entity> size)
{
  int a = 0;
  if (!current.empty()) {
    root.push_back(current);
  }
  current = nlohmann::json::array();
  current.push_back(size); // first element of each array keeps the amount of elements.
}

void
NJSONOutputArchive::operator()(entt::entity entity)
{
  // Here it is assumed that no custom entt-type is chosen
  current.push_back((uint32_t)entity);
}

void
NJSONOutputArchive::close()
{
  if (!current.empty()) {
    root.push_back(current);
  }
}

const std::string
NJSONOutputArchive::as_string()
{
  std::string output = root.dump();
  return output;
}

const std::vector<uint8_t>
NJSONOutputArchive::as_bson()
{
  std::vector<std::uint8_t> as_bson = nlohmann::json::to_bson(root);
  return as_bson;
}

//
// Input Archive
//

NJSONInputArchive::NJSONInputArchive(const std::string& json_string)
{
  root = nlohmann::json::parse(json_string);
};

NJSONInputArchive::~NJSONInputArchive() {}

void
NJSONInputArchive::next_root()
{
  root_idx++;
  if (root_idx >= root.size()) {
    // ERROR
    return;
  }
  current = root[root_idx];
  current_idx = 0;
}

void
NJSONInputArchive::operator()(std::underlying_type_t<entt::entity>& s)
{
  next_root();
  int size = current[0].get<int>();
  current_idx++;
  s = (std::underlying_type_t<entt::entity>)size; // pass amount to entt
}

void
NJSONInputArchive::operator()(entt::entity& entity)
{
  uint32_t ent = current[current_idx].get<uint32_t>();
  entity = entt::entity(ent);
  current_idx++;
}

}; // namespace game2d