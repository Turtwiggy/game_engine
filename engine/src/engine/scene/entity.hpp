#pragma once

#include "engine/scene/scene.hpp"

#include <entt/entt.hpp>

namespace fightingengine {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			assert(!HasComponent<T>()); //"Entity already has component!");
			return scene->m_Registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			assert(HasComponent<T>()); //, "Entity does not have component!");
			return scene->m_Registry.get<T>(entity_handle);
		}

		template<typename T>
		bool HasComponent()
		{
			return scene->m_Registry.has<T>(entity_handle);
		}

		template<typename T>
		void RemoveComponent()
		{
			assert(HasComponent<T>()); //, "Entity does not have component!");
			scene->m_Registry.remove<T>(entity_handle);
		}

		operator bool() const { return entity_handle != entt::null; }
		operator uint32_t() const { return (uint32_t)entity_handle; }

		bool operator==(const Entity& other) const
		{
			return entity_handle == other.entity_handle && scene == other.scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	private:
		entt::entity entity_handle{ entt::null };
		Scene* scene = nullptr;
	};

}