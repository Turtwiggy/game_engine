#pragma once

#include "engine/entt/helpers.hpp"
#include "modules/events/events_components.hpp"
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>
#include <entt/entt.hpp>

namespace game2d {

template<class A, class B>
std::pair<entt::entity, entt::entity>
collision_of_interest(const entt::registry& r, const entt::entity& a, const entt::entity& b)
{
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
    const entt::entity a = (entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
    const entt::entity b = (entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

    auto& evts = get_first_component<SINGLE_Events>(r);
    OnCollisionEnter evt;
    evt.a = a;
    evt.b = b;
    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();
  }

  void EndContact(b2Contact* contact) override
  {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    const entt::entity a = (entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer;
    const entt::entity b = (entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer;

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