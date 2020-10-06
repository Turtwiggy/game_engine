#pragma once

#include "engine/scene/scene.hpp"
#include "engine/scene/entity.hpp"

#include <memory>

using namespace fightingengine;

namespace game_3d
{

    class SceneHierarchyPanel
    {
    public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const std::shared_ptr<Scene>& scene);

		void set_context(const std::shared_ptr<Scene>& scene);

		void on_imgui_render();
	private:
		void draw_entity_node(Entity entity);
		void draw_components(Entity entity);
	private:
		std::shared_ptr<Scene> context;
		Entity selection_context;
    };

}