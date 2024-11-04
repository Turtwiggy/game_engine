#pragma once

#include "actors/actor_helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/system_ai/system_ai_maths.hpp"

#include <SDL2/SDL_log.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <format>
#include <memory>
#include <optional>

namespace game2d {

struct Consideration
{
  virtual ~Consideration() = default;
  virtual float Evaluate(entt::registry& r, entt::entity e) const = 0;
};

struct AmmoConsideration : public Consideration
{
  float Evaluate(entt::registry& r, entt::entity e) const override
  {
    // var ammo = ecs.GetComponent<AmmoComponent>(e);
    // float percent01 = ammo.cur / (float)ammo.max;

    // response curve model
    //
    // for ammo, use a negative exponential
    // because the less ammo we have,
    // the more important it becomes to reload

    // var utility = WiggyMath.ExponentialDecay(percent01);
    // // Debug.Log("(ammo) " + utility);

    // return Mathf.Clamp01(utility);
    float utility = 0.0f;

    return std::clamp(utility, 0.0f, 1.0f);
  };
};

struct HealConsideration : public Consideration
{
  float Evaluate(entt::registry& r, entt::entity e) const override
  {
    auto hp_c = r.get<HealthComponent>(e);
    float percent01 = hp_c.hp / (float)hp_c.max_hp;

    // response curve model
    //
    // for health, use a logistic function
    // because it's ok to have some damage taken,
    // but the more damage we take, the more urgent it becomes to heal
    auto utility = Logistic(percent01);

    return std::clamp(utility, 0.0f, 1.0f);
  }
};

struct MoveConsiderationData
{
  std::vector<glm::ivec2> final_path;
};
struct MoveConsideration : public Consideration
{
  float Evaluate(entt::registry& r, entt::entity e) const override;
};

struct AttackConsiderationData
{
  std::vector<entt::entity> targets;
};
struct AttackConsideration : public Consideration
{
  float Evaluate(entt::registry& r, entt::entity e) const override;
};

//
//
//

enum class ActionEnum
{
  NONE,
  MOVE,
  SHOOT,
  USE_ITEM,
};

struct Action
{
  std::vector<std::shared_ptr<Consideration>> considerations;

  Action() = default;
  virtual ~Action() = default;

  float Evaluate(entt::registry& r, entt::entity e)
  {
    float score = 0.0f;
    float EPSILON = 0.01f;

    if (considerations.size() != 0)
      score = 1.0f;

    for (const auto& c : considerations)
      score *= c->Evaluate(r, e);

    if (score - EPSILON > 0.0f)
      return score;

    return 0.0f;
  };

  virtual std::string GetClassName() const { return "DefaultAction"; }
  virtual ActionEnum GetActionEnum() const { return ActionEnum::NONE; }
};

struct MoveAction : public Action
{
  std::string GetClassName() const override { return "MoveAction"; }
  ActionEnum GetActionEnum() const override { return ActionEnum::MOVE; }

  MoveAction()
  {
    //
    considerations.push_back(std::make_shared<MoveConsideration>());
  }
};

struct AttackAction : public Action
{
  std::string GetClassName() const override { return "AttackAction"; }
  ActionEnum GetActionEnum() const override { return ActionEnum::SHOOT; }

  AttackAction()
  {
    //
    considerations.push_back(std::make_shared<AttackConsideration>());
  };
};

// A simple 3 state FSM for AI.
// IDLE: accept new tasks
// REASONING: thinking time to seem more real
// MOVE: moving physically in world
// ANIMATE: an animation is playing, which triggers events e.g. sound
enum class BRAIN_STATE
{
  IDLE,
  REASONING,
  MOVE,
  ANIMATE,
};

struct DefaultBrainComponent
{
  std::vector<std::shared_ptr<Action>> actions;
  BRAIN_STATE brain_fsm = BRAIN_STATE::IDLE;

  DefaultBrainComponent()
  {
    actions.push_back(std::make_shared<MoveAction>());
    actions.push_back(std::make_shared<AttackAction>());
  }
};

class Reasoner
{
public:
  static std::optional<std::shared_ptr<Action>> Evaluate(entt::registry& r, entt::entity e, DefaultBrainComponent& b)
  {
    std::map<float, std::shared_ptr<Action>> sorted_actions;

    // Evaluate all actions
    for (auto& a : b.actions) {
      float score = a->Evaluate(r, e);
      SDL_Log("%s", std::format("action: {}, score: {}", a->GetClassName(), score).c_str());

      if (score > 0.0f)
        sorted_actions.emplace(score, a);
    }

    std::optional<std::shared_ptr<Action>> result = std::nullopt;

    // take the action with the highest utility (value at end of map)
    if (sorted_actions.size() > 0)
      result = sorted_actions.rend()->second;

    return result;
  };
};

} // namespace game2d