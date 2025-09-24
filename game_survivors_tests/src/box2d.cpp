#include "pch.hpp"

#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/system.hpp"
#include "resources/resources.hpp"

namespace game2d {

namespace tests {

TEST(TestSuite, TextBox2DUserData)
{
  // arrange
  entt::registry r;
  emplace_or_replace_physics_world(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  SINGLE_RendererInfo::instance = get_default_textures();
  const auto& ri = SINGLE_RendererInfo::instance;
  SINGLE_Animations anims;
  for (const auto& tex : ri.user_textures)
    load_sprites(anims, tex);
  SINGLE_Animations::instance = anims;

  entt::entity body_eid = entt::null;
  entt::entity shape_eid = entt::null;
  entt::entity e_copy = entt::null;
  entt::entity shape_copy = entt::null;

  // act
  // note: in a scope to make sure that e and shape_e go out of scope
  {
    const auto e = spawn(r, "item_xp");
    give_life(r, e, { 0, 0 });
    const auto& body_c = r.get<PhysicsBodyComponent>(e);
    const auto body_id = body_c.bodyId;
    const auto shape_ids = body_c.fixtures;
    const entt::entity shape_e = body_c.fixtures[0];
    const auto& fixture_c = r.get<PhysicsFixtureComponent>(shape_e);
    auto* body_data = b2Body_GetUserData(body_id);
    auto* shape_data = b2Shape_GetUserData(fixture_c.shapeId);
    body_eid = (entt::entity)(reinterpret_cast<uintptr_t>(body_data));
    shape_eid = (entt::entity)(reinterpret_cast<uintptr_t>(shape_data));
    e_copy = e;
    shape_copy = shape_e;
  }

  // assert
  ASSERT_TRUE(e_copy == body_eid);      // check the body user data has the body entity id
  ASSERT_TRUE(shape_copy == shape_eid); // check the shape user data has the shape entity id
};

} // namespace tests

} // namespace game2d