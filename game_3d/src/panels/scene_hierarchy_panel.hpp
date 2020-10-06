#pragma once

#include "engine/scene/scene.hpp"
#include "engine/scene/entity.hpp"

using namespace fightingengine;

namespace game_3d
{

    class SceneHierarchyPanel
    {
    public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
    };

}