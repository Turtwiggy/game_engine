#include "pch.hpp"

#include "snake_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "snake_components.hpp"

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
  dead_c.dead.push_back(snake_c.target_e_0);
  dead_c.dead.push_back(snake_c.target_e_1);

  // Kill all sections when head dies?
  entt::entity e = dead_e;
  while (auto* children_c = r.try_get<HasChildrenComponent>(e)) {
    dead_c.dead.push_back(e);
    if (children_c->children.empty())
      break;
    e = children_c->children[0]; // note: assuming only 1 child
    dead_c.dead.push_back(e);
  }
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
    snake_c.snake_segment_size.x = 16 * 4;
    snake_c.snake_segment_size.y = 16 * 4;
    snake_c.distance_between_segment_pixels = snake_c.snake_segment_size.x;

    // create targets
    snake_c.target_e_0 = spawn(r, "empty");
    r.get<TagComponent>(snake_c.target_e_0).tag = "snake_target_e_0";
    give_life(r, snake_c.target_e_0, { 100, 100 }, { 0, 0 });

    snake_c.target_e_1 = spawn(r, "empty");
    r.get<TagComponent>(snake_c.target_e_1).tag = "snake_target_e_1";
    give_life(r, snake_c.target_e_1, { -100, -100 }, { 0, 0 });

    r.emplace<SnakeData>(segment_e, snake_c);
  }

  auto& snake_c = get_first_component<SnakeData>(r);

  const auto sprite_size = 2.0f * snake_c.snake_segment_size;
  const auto offscreen_pos = glm::vec2{ 2000, 2000 }; // spawn the snake offscreen
  const auto pos = glm::vec2{ offscreen_pos.x + idx * snake_c.distance_between_segment_pixels, offscreen_pos.y };
  give_life(r, segment_e, pos, snake_c.snake_segment_size);
  set_position(r, segment_e, pos);
  set_colour(r, segment_e, { 255, 255, 255, 255 });
  set_size(r, segment_e, sprite_size);
  r.get<DefaultSizeComponent>(segment_e).size = sprite_size;

  if (previous_e != entt::null) {
    r.emplace<HasParentComponent>(segment_e, HasParentComponent{ .parent = previous_e, .destroy_parent_on_cleanup = false });
    r.emplace<HasChildrenComponent>(previous_e, HasChildrenComponent{ { segment_e } });
  }

  // allow neck bone to bend more?
  std::vector<float> segment_bending(snake_c.snake_segments, 30.0f);
  segment_bending[0] = 30;
  segment_bending[1] = 30;

  //
  // Connect with Physics Joints
  //

  const float hp = type == SectionType::HEAD ? head_hp : segment_hp;

  if (type == SectionType::HEAD) {
    // the head follows a dynamic target
    ApplyForceToDynamicTarget target_c;
    target_c.orbit = false;
    r.emplace<ApplyForceToDynamicTarget>(segment_e, target_c);
    r.emplace<PhysicsDynamicTarget>(segment_e, snake_c.target_e_0);
  }

  if (type == SectionType::BODY || type == SectionType::TAIL) {

    const auto body_id_a = r.get<PhysicsBodyComponent>(previous_e).bodyId;
    const auto body_id_b = r.get<PhysicsBodyComponent>(segment_e).bodyId;
    b2Body_SetLinearDamping(body_id_b, 5.0f);
    b2Body_SetFixedRotation(body_id_b, false);

    // create a distance constraint...
    const b2Vec2 pivot1 = b2Body_GetPosition(body_id_a);
    const b2Vec2 pivot2 = b2Body_GetPosition(body_id_b);
    b2DistanceJointDef def = b2DefaultDistanceJointDef();
    def.hertz = 1.0f;        // this should be less than half of the frequency of the timestep
    def.dampingRatio = 1.0f; // typically between 0 and 1. at 1 oscillations should vanish
    def.bodyIdA = body_id_a;
    def.bodyIdB = body_id_b;
    def.localAnchorA = b2Body_GetLocalPoint(def.bodyIdA, pivot1);
    def.localAnchorB = b2Body_GetLocalPoint(def.bodyIdB, pivot2);
    def.enableLimit = true;
    def.minLength = pixels_to_meters(snake_c.distance_between_segment_pixels);
    def.maxLength = pixels_to_meters(snake_c.distance_between_segment_pixels);
    b2CreateDistanceJoint(physics_c.worldId, &def);

    // create a revolute joint...
    b2RevoluteJointDef rdef = b2DefaultRevoluteJointDef();
    rdef.bodyIdA = body_id_a;
    rdef.bodyIdB = body_id_b;
    rdef.localAnchorA = b2Body_GetLocalPoint(body_id_a, b2Body_GetPosition(body_id_a));
    rdef.localAnchorB = b2Body_GetLocalPoint(body_id_b, b2Body_GetPosition(body_id_a)); // rel to a
    rdef.enableLimit = true;
    rdef.lowerAngle = -segment_bending[idx - 1] * engine::Deg2Rad;
    rdef.upperAngle = segment_bending[idx - 1] * engine::Deg2Rad;
    b2CreateRevoluteJoint(physics_c.worldId, &rdef);
  };

  // Allow rotation
  auto& pb_c = r.get<PhysicsBodyComponent>(segment_e);
  b2Body_SetFixedRotation(pb_c.bodyId, false);
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
  }

  // Iterate long the snake and set all the sprites.
  const std::vector<std::string> snake_sprites = {
    "AX_SNAKE_TAIL",              //
    "AX_SNAKE_BODY_SMALL",        //
    "AX_SNAKE_BODY_MED_TO_SMALL", //
    "AX_SNAKE_BODY_MED",          //
    "AX_SNAKE_BODY_BIG_TO_MED",   //
    "AX_SNAKE_BODY_BIG",          //
    "AX_SNAKE_HEAD",              //
  };

  auto tail_e = head_e;
  for (int i = 0; i < snake_c.snake_segments; i++) {
    auto* child_c = r.try_get<HasChildrenComponent>(tail_e);
    if (!child_c)
      break;
    tail_e = child_c->children[0]; // note: assuming only 1 child
  }
  assert(r.get<TagComponent>(tail_e).tag == "actor_snake_tail");

  // now we have the tail... iterate back up, setting the sprites.
  auto segment_e = tail_e;
  for (int i = 0; i < snake_c.snake_segments; i++) {

    if (i < snake_sprites.size() - 1)
      set_sprite(r, segment_e, snake_sprites[i]);
    else
      set_sprite(r, segment_e, "AX_SNAKE_BODY_BIG");

    auto* has_parent_c = r.try_get<HasParentComponent>(segment_e);
    if (!has_parent_c) {
      set_sprite(r, segment_e, "AX_SNAKE_HEAD");
      continue;
    }
    segment_e = has_parent_c->parent;
  }

  return head_e;
};

void
update_actor_snake(entt::registry& r, glm::vec2 mouse_pos, float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SnakeData, r, snake_e, snake_c);

#if defined(_DEBUG)
  // imgui_draw_float("circle_speed", circle_speed);
  // imgui_draw_float("snake_speed", snake_speed);
  // ImGui::SliderFloat("circle_radius", &circle_radius, 0, 1000.0f);
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
  auto view = r.view<ActorSpeedComponent, PhysicsDynamicTarget, const BossComponent>();
  auto segments = r.view<SnakeSegment>();
  for (const auto& [e, speed_c, target_c, boss_c] : view.each()) {

    // -2 because head and tail curently dont have SnakeSegment attached
    float percent = (int)segments.size() / (float)(snake_c.snake_segments - 2);

    // slow the snake down as the segments die
    speed_c.base_speed = percent * snake_speed;
    speed_c.current_speed = percent * snake_speed;
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

    if (children_c->children.size() == 0)
      break;

    auto next_e = children_c->children[0]; // note: assuming only 1 child
    if (!r.valid(next_e) || next_e == entt::null)
      break;

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