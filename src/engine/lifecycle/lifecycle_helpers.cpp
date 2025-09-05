#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "lifecycle_helpers.hpp"

namespace game2d {

bool
is_parent_valid(entt::registry& r, entt::entity e)
{
  auto* parent_c = r.try_get<HasParentComponent>(e);
  if (!parent_c)
    return false;

  if (!r.valid(parent_c->parent) || parent_c->parent == entt::null)
    return false;

  return true;
};

// Something with HasChildrenComponent was destroyed.
void
on_parent_destroyed(entt::registry& r, const entt::entity e)
{
  // When a parent is destroyed, clean up it's children.

  auto* children_c = r.try_get<HasChildrenComponent>(e);
  if (children_c == nullptr)
    return;

#if defined(_DEBUG)
  const auto& tag_c = r.get<TagComponent>(e);
  SDL_Log("%s died with children. (%i)", tag_c.tag.c_str(), children_c->children.size());
#endif

  auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
  for (const auto child_e : children_c->children) {
    SDL_Log("Adding child %zu to dead list.", static_cast<uint32_t>(child_e));
    dead_c.dead.push_back(child_e);
  }
}

// Something with HasParentComponent was destroyed.
void
on_child_destroyed(entt::registry& r, const entt::entity e)
{
  // When a child is destroyed, remove it from it's parent's children list.

  std::string tag = "unknown";
  if (r.all_of<TagComponent>(e)) {
    const auto& c_tag_c = r.get<TagComponent>(e);
    tag = c_tag_c.tag;
  }

  auto* parent_c = r.try_get<HasParentComponent>(e);
  if (parent_c == nullptr || !r.valid(parent_c->parent)) {
    SDL_Log("%s", std::format("Invalid parent for child({}) {}", tag.c_str(), (uint32_t)parent_c->parent).c_str());
    return;
  }

#if defined(_DEBUG)
  const auto& p_tag_c = r.get<TagComponent>(parent_c->parent);
  SDL_Log("%s",
          std::format("child ({})({}) died with a parent ({})({})",
                      tag.c_str(),
                      (uint32_t)e,
                      p_tag_c.tag.c_str(),
                      (uint32_t)parent_c->parent)
            .c_str());

  if (strcmp(tag.c_str(), "fixture_item") == 0) {
    int k = 1;
  }
#endif

  // If you're a fixture, add your parent to the dead list.
  // why doesnt PhysicsFixtureComponent exist on e?
  if (r.all_of<PhysicsFixtureComponent>(e)) {
    SDL_Log(
      "%s",
      std::format("Child was a fixture, adding parent({}) to dead list.", static_cast<uint32_t>(parent_c->parent).c_str()));
    auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
    dead_c.dead.push_back(parent_c->parent);
  } else if (r.all_of<PhysicsBodyComponent>(parent_c->parent)) {
    SDL_Log("Child had a PhysicsBodyComponent parent(%zu) to dead list.", static_cast<uint32_t>(parent_c->parent));
    auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
    dead_c.dead.push_back(parent_c->parent);
  }

  // Remove the child from the parent
  auto* children_c = r.try_get<HasChildrenComponent>(parent_c->parent);
  if (children_c == nullptr)
    return; // e.g. bullets that the weapon doesnt keep track of
  auto& c = children_c->children;
  c.erase(std::remove(c.begin(), c.end(), e), c.end());
  SDL_Log("Child removed itself from parent, new_size: %i", c.size());

  // if (c.empty())
  //   r.remove<HasChildrenComponent>(parent_c->parent);
}

} // namespace game2d