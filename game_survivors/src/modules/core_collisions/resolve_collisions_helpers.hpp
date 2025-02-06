#pragma once

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core_events/events_components.hpp"

#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>
#include <entt/entt.hpp>

namespace game2d {

enum class FixtureOrBody
{
  FIXTURE,
  BODY,
};

template<class A, class B>
std::pair<entt::entity, entt::entity>
collision_of_interest(entt::registry& r,
                      entt::entity fixture_a,
                      entt::entity fixture_b,
                      FixtureOrBody fob_a,
                      FixtureOrBody fob_b)
{
  // type A should exist on the fixture level
  // type B should exist on the body level
  if (fob_a == FixtureOrBody::FIXTURE && fob_b == FixtureOrBody::BODY) {

    const auto* a_has_type_a = r.try_get<A>(fixture_a);
    const auto* b_has_type_a = r.try_get<A>(fixture_b);

    if (a_has_type_a != nullptr) {
      // type A found on the fixture level on entity a
      // check the body level on entity b.
      if (auto* b_parent = r.try_get<HasParentComponent>(fixture_b)) {
        auto* b_has_type_b = r.try_get<B>(b_parent->parent);
        if (b_has_type_b)
          // entity a has the comp on the fixture level,
          // entity b has the comp on the body level
          // note: this returns fixture e, body e
          return { fixture_a, b_parent->parent };
      }
    }

    if (b_has_type_a != nullptr) {
      // type A found on the fixture level on entity b.
      // check the body level on entity a.
      if (auto* a_parent = r.try_get<HasParentComponent>(fixture_a)) {
        auto* a_has_type_b = r.try_get<B>(a_parent->parent);
        if (a_has_type_b)
          // entity b has the comp on the fixture level,
          // entity a has the comp on the body level;
          // note: this returns fixture e, body e
          return { fixture_b, a_parent->parent };
      }
    }
  }

  if (fob_a == FixtureOrBody::BODY && fob_b == FixtureOrBody::FIXTURE) {
    SDL_Log("(Body)(Fixture) not impl, but (Fixture)(Body) is");
    exit(1);
  }

  entt::entity a = entt::null;
  entt::entity b = entt::null;

  if (fob_a == FixtureOrBody::BODY && fob_b == FixtureOrBody::BODY) {
    a = r.get<HasParentComponent>(fixture_a).parent;
    b = r.get<HasParentComponent>(fixture_b).parent;
  }

  if (fob_a == FixtureOrBody::FIXTURE && fob_b == FixtureOrBody::FIXTURE) {
    a = fixture_a;
    b = fixture_b;
  }

  {
    const auto* a_has_type_a = r.try_get<A>(a);
    const auto* b_has_type_b = r.try_get<B>(b);
    if (a_has_type_a != nullptr && b_has_type_b != nullptr)
      return { a, b };
  }
  {
    const auto* a_has_type_b = r.try_get<B>(a);
    const auto* b_has_type_a = r.try_get<A>(b);
    if (a_has_type_b != nullptr && b_has_type_a != nullptr)
      return { b, a };
  }
  return { entt::null, entt::null };
};

struct OnCollisionEnter
{
  entt::entity a = entt::null;
  entt::entity b = entt::null;
};

struct OnCollisionExit
{
  entt::entity a = entt::null;
  entt::entity b = entt::null;
};

//
// Overwrite the box2d contact listener, and this gets set on the world
//
class PhysicsEvents : public b2ContactListener
{
public:
  PhysicsEvents(entt::registry& registry)
    : r(registry) {};

  // Implement BeginContact and delegate to all listeners
  void BeginContact(b2Contact* contact) override
  {
    // Handle "collision enter" logic here
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    if (fixtureA->GetBody() == fixtureB->GetBody()) {
      int k = 1;
      return;
    }

    const entt::entity a = (entt::entity)fixtureA->GetUserData().pointer;
    const entt::entity b = (entt::entity)fixtureB->GetUserData().pointer;

    auto& evts = get_first_component<SINGLE_Events>(r);
    OnCollisionEnter evt;
    evt.a = a;
    evt.b = b;
    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();

    // SDL_Log("(Enter) %s, %s", r.get<TagComponent>(a).tag.c_str(), r.get<TagComponent>(b).tag.c_str());
  }

  void EndContact(b2Contact* contact) override
  {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    const entt::entity a = (entt::entity)fixtureA->GetUserData().pointer;
    const entt::entity b = (entt::entity)fixtureB->GetUserData().pointer;

    auto& evts = get_first_component<SINGLE_Events>(r);
    OnCollisionExit evt;
    evt.a = a;
    evt.b = b;
    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();
  }

private:
  entt::registry& r;
};

//
// A wrapper around the Listener letting
// entt request this component to get the listener
//
class SINGLE_PhysicsEvents
{
public:
  PhysicsEvents* listener;
};

} // namespace game2d