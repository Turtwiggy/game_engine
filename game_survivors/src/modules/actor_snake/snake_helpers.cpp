#include "pch.hpp"

#include "snake_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "snake_components.hpp"

#include <box2d/b2_distance_joint.h>
#include <box2d/b2_math.h>
#include <box2d/b2_revolute_joint.h>

namespace game2d {

// loosely based on:
// https://youtu.be/qlfh_rv6khY?t=45

enum class SectionType
{
  HEAD,
  BODY,
  TAIL,
};

static float circle_speed = 0.2f;
static float snake_speed = 90.0f;
static float circle_radius = 450.0f;
const float time_between_bullets = 2.0f;
const float head_hp = 600;
const float segment_hp = 400;

auto cleanup_on_death = [](entt::registry& r, entt::entity dead_e) {
  auto& snake_c = r.get<SnakeData>(dead_e);
  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);

  // Remove targets when head dies
  dead_c.dead.emplace(snake_c.target_e_0);
  dead_c.dead.emplace(snake_c.target_e_1);

  // Kill all sections when head dies?
  entt::entity e = dead_e;
  while (auto* children_c = r.try_get<HasChildrenComponent>(e)) {
    dead_c.dead.emplace(e);
    e = children_c->children[0]; // note: assuming only 1 child
    dead_c.dead.emplace(e);
  }
};

auto segment_died = [](entt::registry& r, entt::entity segment_e) {
  auto snake_c = get_first_component<SnakeData>(r);

  // child dead; inform parent. oh no.
  if (auto* parent_c = r.try_get<HasParentComponent>(segment_e))
    r.remove<HasChildrenComponent>(parent_c->parent);

  SDL_Log("Segment died...");
};

entt::entity
create_segment(entt::registry& r, const SectionType type, entt::entity previous_e, int idx)
{
  auto& physics_c = get_first_component<SINGLE_Physics>(r);

  entt::entity segment_e = entt::null;
  if (type == SectionType::HEAD) {
    segment_e = spawn(r, "actor_snake_head");
  }
  if (type == SectionType::BODY) {
    segment_e = spawn(r, "actor_snake_body");
    r.emplace<SnakeSegment>(segment_e);
    r.emplace<CooldownComponent>(segment_e, CooldownComponent{ .time_max = time_between_bullets });
  }
  if (type == SectionType::TAIL)
    segment_e = spawn(r, "actor_snake_tail");

  if (segment_e == entt::null) {
    throw std::runtime_error("segment_e is entt::null");
    exit(1); // crash
  }

  if (type == SectionType::HEAD) {
    SnakeData snake_c;

    // create targets
    snake_c.target_e_0 = spawn(r, "empty");
    give_life(r, snake_c.target_e_0, { 100, 100 }, { 0, 0 });
    snake_c.target_e_1 = spawn(r, "empty");
    give_life(r, snake_c.target_e_1, { -100, -100 }, { 0, 0 });

    r.emplace<SnakeData>(segment_e, snake_c);
  }

  auto& snake_c = get_first_component<SnakeData>(r);

  const auto offscreen_pos = glm::vec2{ 2000, 2000 }; // todo: probably fix this
  const auto pos = glm::vec2{ offscreen_pos.x + idx * snake_c.distance_betwee_segment_pixels, offscreen_pos.y };
  give_life(r, segment_e, pos, snake_c.snake_segment_size);
  set_position(r, segment_e, pos);
  set_colour(r, segment_e, { 255, 255, 255, 255 });
  if (previous_e != entt::null) {
    r.emplace<HasParentComponent>(segment_e, previous_e);
    r.emplace<HasChildrenComponent>(previous_e, HasChildrenComponent{ { segment_e } });
  }

  // allow neck bone to bend more?
  std ::vector<float> segment_bending(10.0f, snake_c.snake_segments);
  segment_bending[0] = 20;
  segment_bending[1] = 10;

  //
  // Connect with Physics Joints
  //

  const float hp = type == SectionType::HEAD ? head_hp : segment_hp;

  if (type == SectionType::HEAD) {
    // the head follows a dynamic target
    ApplyForceToDynamicTarget target_c;
    target_c.speed = 5.0f;
    target_c.orbit = false;
    r.emplace<ApplyForceToDynamicTarget>(segment_e, target_c);
    r.emplace<PhysicsDynamicTarget>(segment_e, snake_c.target_e_0);
  }

  if (type == SectionType::BODY || type == SectionType::TAIL) {

    auto& body_a = r.get<PhysicsBodyComponent>(previous_e).body;
    auto& body_b = r.get<PhysicsBodyComponent>(segment_e).body;
    body_b->SetLinearDamping(5.0f);
    body_b->SetFixedRotation(false);

    // create a distance constraint...
    b2DistanceJointDef def;
    def.Initialize(body_a, body_b, body_a->GetWorldCenter(), body_b->GetWorldCenter());
    def.minLength = pixels_to_meters(snake_c.distance_betwee_segment_pixels);
    def.maxLength = pixels_to_meters(snake_c.distance_betwee_segment_pixels);
    float frequencyHz = 1.0f;  // this should be less than half of the frequency of the timestep
    float dampingRatio = 1.0f; // typically between 0 and 1. at 1 oscillations should vanish
    b2LinearStiffness(def.stiffness, def.damping, frequencyHz, dampingRatio, def.bodyA, def.bodyB);
    physics_c.world->CreateJoint(&def);

    // create a revolute joint...
    b2RevoluteJointDef rdef;
    rdef.Initialize(body_a, body_b, body_a->GetWorldCenter());
    rdef.enableLimit = true;
    rdef.lowerAngle = -segment_bending[idx - 1] * engine::Deg2Rad;
    rdef.upperAngle = segment_bending[idx - 1] * engine::Deg2Rad;
    physics_c.world->CreateJoint(&rdef);
  }

  // Allow rotation
  auto& pb_c = r.get<PhysicsBodyComponent>(segment_e);
  pb_c.body->SetFixedRotation(false);
  r.emplace<SetTransformRotationBasedOnPhysicsVelocity>(segment_e);

  // Add HP
  auto fixture_e = get_fixture_by_tag(r, segment_e, "fixture_core");
  r.emplace<HealthComponent>(fixture_e, HealthComponent{ .max_hp = hp, .hp = hp });
  r.emplace<EnemyComponent>(fixture_e); // duplicate enemy component on fixture?
  r.emplace<EnemyComponent>(segment_e);
  r.emplace<TeamComponent>(segment_e, TeamComponent{ AvailableTeams::enemy });

  return segment_e;
};

entt::entity
create_snake(entt::registry& r)
{
  auto head_e = create_segment(r, SectionType::HEAD, entt::null, 0);
  r.emplace<BossComponent>(head_e); // make it a boss
  auto& head_death_callbacks = r.get_or_emplace<OnDeathCallbacks>(head_e);
  head_death_callbacks.callbacks.push_back(cleanup_on_death);

  entt::entity prv_section_e = head_e;

  SnakeData snake_c;
  for (int i = 1; i < snake_c.snake_segments; i++) {
    snake_c = get_first_component<SnakeData>(r);

    if (i == snake_c.snake_segments - 1)
      prv_section_e = create_segment(r, SectionType::TAIL, prv_section_e, i);
    else
      prv_section_e = create_segment(r, SectionType::BODY, prv_section_e, i);

    auto& on_death_callbacks_c = r.get_or_emplace<OnDeathCallbacks>(prv_section_e);
    on_death_callbacks_c.callbacks.push_back(segment_died);
  }

  return head_e;
};

void
update_snake(entt::registry& r, glm::vec2 mouse_pos, float dt)
{
  GET_FIRST_OR_RETURN(SnakeData, r, snake_e, snake_c);

#if defined(_DEBUG)
  imgui_draw_float("circle_speed", circle_speed);
  imgui_draw_float("snake_speed", snake_speed);
  ImGui::SliderFloat("circle_radius", &circle_radius, 0, 1000.0f);
  // bool slider_changed = ImGui::SliderInt("target", &target, 0, 2);
#endif

  static int target = 0;
  static float angle_cw = 0.0f;
  static float angle_1 = 0.0f;
  angle_cw += dt * circle_speed;
  angle_cw = engine::clamp_axis(angle_cw);
  angle_1 += dt * circle_speed;
  angle_1 = engine::clamp_axis(angle_1);

  // target pos 1...
  const auto dir = engine::angle_radians_to_direction(angle_cw);
  const auto pos = engine::ray_at({ .origin = { 0, 0, 0 }, .dir = { dir.x, dir.y, 0.0f } }, circle_radius);
  set_position(r, snake_c.target_e_0, { pos.x, pos.y });

  // target pos 2...
  const auto dir1 = engine::angle_radians_to_direction(angle_1 - engine::PI);
  const auto pos1 = engine::ray_at({ .origin = { 0, 0, 0 }, .dir = { dir1.x, dir1.y, 0.0f } }, circle_radius);
  set_position(r, snake_c.target_e_1, { pos1.x, pos1.y });

  // adjust the snake speed
  auto view = r.view<ApplyForceToDynamicTarget, PhysicsDynamicTarget, BossComponent>();
  auto segments = r.view<SnakeSegment>();
  for (const auto& [e, force_c, target_c, boss_c] : view.each()) {

    // -2 because head and tail curently dont have SnakeSegment attached
    float percent = (int)segments.size() / (float)(snake_c.snake_segments - 2);

    // slow the snake down as the segments die
    force_c.speed = percent * snake_speed;
  }

  // swap targets every X seconds
  snake_c.snake_timer += dt;
  if (snake_c.snake_timer >= snake_c.snake_time_to_swap_points) {
    snake_c.snake_timer = 0.0f;
    for (const auto& [e, force_c, target_c, boss_c] : view.each())
      target_c.target = target_c.target == snake_c.target_e_0 ? snake_c.target_e_1 : snake_c.target_e_0;
  }

  // set snake rotations
  entt::entity e = snake_e;
  while (auto* children_c = r.try_get<HasChildrenComponent>(e)) {
    auto next_e = children_c->children[0]; // note: assuming only 1 child
    if (!r.valid(next_e) || next_e == entt::null)
      continue;

    const auto cur_pos = get_position(r, e);
    const auto nxt_pos = get_position(r, next_e);

    const auto raw_dir = nxt_pos - cur_pos;
    const auto angle = engine::dir_to_angle_radians(raw_dir);
    auto& transform_c = r.get<TransformComponent>(next_e);
    transform_c.rotation_radians.z = angle - engine::PI;

    e = next_e;
  }
};

} // namespace game2d