#pragma once

//c++ standard library headers
#include <memory>

//yoru project headers
#include "engine/scene/entity.hpp"
#include "engine/scene/scene.hpp"

namespace game_3d
{

    class SceneHierarchyPanel
    {
    public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const std::shared_ptr<fightingengine::Scene>& scene);

		void set_context(const std::shared_ptr<fightingengine::Scene>& scene);

		void on_imgui_render();
	private:
		void draw_entity_node(fightingengine::Entity entity);
		void draw_components(fightingengine::Entity entity);
	private:
		std::shared_ptr<fightingengine::Scene> context;
		fightingengine::Entity selection_context;
    };

} //namespace game_3d