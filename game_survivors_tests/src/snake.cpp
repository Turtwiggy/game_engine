#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/lifecycle/lifecycle_helpers.hpp"
#include "engine/lifecycle/lifecycle_system.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_snake/snake_components.hpp"
#include "modules/actors/actor_snake/snake_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/scene/scene_helpers.hpp"

namespace game2d {

namespace tests {

TEST(TestSuite, CreateSnake)
{
  // setup
  entt::registry r;
  create_persistent<OrthographicCamera>(r);
  create_persistent<InputComponent>(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  r.emplace<TransformComponent>(get_first<OrthographicCamera>(r));
  move_to_scene_start(r, Scene::test);

  // act
  auto head_e = create_snake(r);

  // assert
  ASSERT_EQ(r.view<BossComponent>().size(), 1);
}

TEST(TestSuite, CreateSnake_DestroyHead)
{
  // setup
  entt::registry r;
  create_persistent<OrthographicCamera>(r);
  create_persistent<InputComponent>(r);
  create_persistent<SINGLE_EntityBinComponent>(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  r.emplace<TransformComponent>(get_first<OrthographicCamera>(r));
  move_to_scene_start(r, Scene::test);
  auto head_e = create_snake(r);
  auto fixture_e = get_fixture_by_tag(r, head_e, "fixture_core");
  const auto& head_c = r.get<HealthComponent>(fixture_e); // check it has hp

  // act (kill the snake head)
  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
  dead_c.dead.push_back(head_e);
  update_lifecycle_system(r, 16);

  // assert
  ASSERT_EQ(r.view<BossComponent>().size(), 0);
  ASSERT_EQ(r.view<HealthComponent>().size(), 0);
}

TEST(TestSuite, CreateSnake_DestroyTail)
{
  // setup
  entt::registry r;
  create_persistent<OrthographicCamera>(r);
  create_persistent<InputComponent>(r);
  create_persistent<SINGLE_EntityBinComponent>(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  r.emplace<TransformComponent>(get_first<OrthographicCamera>(r));
  move_to_scene_start(r, Scene::test);
  auto head_e = create_snake(r);

  // get the tail segment.
  auto segment_e = head_e;
  SnakeData default_data;
  for (int i = 0; i < (default_data.snake_segments - 1); i++) {
    auto& children_c = r.get<HasChildrenComponent>(segment_e);
    segment_e = children_c.children[0];
  }

  // Check we've got the tail.
  ASSERT_TRUE(r.get<TagComponent>(segment_e).tag == "actor_snake_tail");

  // act (kill the snake tail)
  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
  dead_c.dead.push_back(segment_e);
  update_lifecycle_system(r, 16);

  // assert
  ASSERT_EQ(r.view<BossComponent>().size(), 1);
  ASSERT_EQ(r.view<HealthComponent>().size(), default_data.snake_segments - 1);
}

// note: destroing a middle section destroys everything after it (additional sections, and the tail)
TEST(TestSuite, CreateSnake_DestroyMiddleSection)
{
  // setup
  entt::registry r;
  create_persistent<OrthographicCamera>(r);
  create_persistent<InputComponent>(r);
  create_persistent<SINGLE_EntityBinComponent>(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  r.on_destroy<HasChildrenComponent>().connect<&on_parent_destroyed>();
  r.on_destroy<HasParentComponent>().connect<&on_child_destroyed>();
  r.emplace<TransformComponent>(get_first<OrthographicCamera>(r));
  move_to_scene_start(r, Scene::test);
  auto head_e = create_snake(r);

  // get the tail segment.
  auto segment_e = head_e;
  SnakeData default_data;
  for (int i = 0; i < (default_data.snake_segments - 5); i++) {
    auto& children_c = r.get<HasChildrenComponent>(segment_e);
    segment_e = children_c.children[0];
  }

  // Check we've got the tail.
  ASSERT_TRUE(r.get<TagComponent>(segment_e).tag != "actor_snake_tail");

  // act (kill the snake section)
  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
  dead_c.dead.push_back(segment_e);
  update_lifecycle_system(r, 16);

  // assert
  ASSERT_EQ(r.view<BossComponent>().size(), 1);
  ASSERT_EQ(r.view<HealthComponent>().size(), default_data.snake_segments - 5);
}

}

}
