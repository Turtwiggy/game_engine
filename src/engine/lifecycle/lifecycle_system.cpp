#include "pch.hpp"

#include "lifecycle_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"

namespace game2d {

template<typename T>
std::vector<T>
remove_duplicates(const std::vector<T>& input)
{
  std::unordered_set<T> seen;
  std::vector<T> result;

  for (const auto& item : input) {
    if (seen.insert(item).second) { // insert returns pair<iterator, bool>
      result.push_back(item);
    }
  }

  return result;
};

void
delete_if_unique(entt::registry& r, std::vector<entt::entity>& deleted, const entt::entity& e)
{
  if (std::find(deleted.begin(), deleted.end(), e) != deleted.end())
    return;
  deleted.push_back(e);

#if defined(_DEBUG)
  // auto tag = r.get<TagComponent>(e).tag.c_str();
  // SDL_Log("destroying: %s (%zu)", tag, (uint32_t)e);
#endif

  r.destroy(e);
};

void
update_lifecycle_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& physics_c = get_first_component<SINGLE_Physics>(r);
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // update all components with timed lifecycle
  const auto view = r.view<EntityTimedLifecycle>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, lifecycle_c] : view.each()) {
    if (lifecycle_c.milliseconds_alive > lifecycle_c.milliseconds_alive_max)
      dead.dead.push_back(e);
    lifecycle_c.milliseconds_alive += static_cast<int>(milliseconds_dt);
  };

  std::vector<entt::entity> deleted;

  while (!dead.dead.empty()) {

    // Note: dead objects can create more dead objects in the callbacks.
    const auto e = dead.dead.front();
    std::erase(dead.dead, e);

    // check that deleted doesnt contain the entity already.
    if (std::find(deleted.begin(), deleted.end(), e) != deleted.end())
      continue;

    // do callbacks.
    if (auto* callback = r.try_get<OnDeathCallbacks>(e))
      for (const auto& cb : callback->callbacks)
        cb(r, e);

    // delete the fixtures.
    if (auto* pb = r.try_get<PhysicsBodyComponent>(e)) {

      // delete the fixture(s), and the body.
      for (const auto fixture_e : pb->fixtures)
        delete_if_unique(r, deleted, fixture_e);

      // remove all occurances of fixtures from dead.dead as they were just deleted.
      dead.dead.erase(std::remove_if(dead.dead.begin(),
                                     dead.dead.end(),
                                     [&](const entt::entity& entity) {
                                       return std::find(pb->fixtures.begin(), pb->fixtures.end(), entity) !=
                                              pb->fixtures.end();
                                     }),
                      dead.dead.end());

      b2DestroyBody(pb->bodyId);
    }

    // if the fixture is in dead.dead,
    // make sure to request to delete the body.
    if (auto* pf = r.try_get<PhysicsFixtureComponent>(e))
      dead.dead.push_back(r.get<HasParentComponent>(e).parent);

    // note: safeguard which objects get deleted, because
    // fixtures can get added to dead.dead.push_back(fixture_e)
    delete_if_unique(r, deleted, e);
  }

#if defined(_DEBUG)
  // auto before_view = r.view<entt::entity>();
  // auto before_count = before_view.size();
#endif

#if defined(_DEBUG)
  // auto after_view = r.view<entt::entity>();
  // auto after_count = before_view.size();
  // assert(after_count == (before_count - uniquely_dead.size()));
#endif

  // process create requests
  const auto requests = r.view<WaitForInitComponent>();
  r.remove<WaitForInitComponent>(requests.begin(), requests.end());
};

} // namespace game2d