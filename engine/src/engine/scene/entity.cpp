#include "engine/scene/entity.hpp"

namespace fightingengine {

    Entity::Entity(entt::entity h, Scene* s)
		: entity_handle(h), scene(s)
	{
	}

}