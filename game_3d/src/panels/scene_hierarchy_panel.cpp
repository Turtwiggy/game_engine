#include "panels/scene_hierarchy_panel.hpp"

#include "engine/scene/components.hpp"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace game_3d {

	SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& context)
	{
		set_context(context);
	}

	void SceneHierarchyPanel::set_context(const std::shared_ptr<Scene>& c)
	{
		context = c;
	}

	void SceneHierarchyPanel::on_imgui_render()
	{
		ImGui::Begin("Scene Hierarchy");

		context->get_registry().each([&](auto entityID)
		{
			Entity entity{ entityID , context.get() };
			draw_entity_node(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			selection_context = {};

		ImGui::End();

		ImGui::Begin("Properties");
		if (selection_context)
			draw_components(selection_context);

		ImGui::End();
	}

	void SceneHierarchyPanel::draw_entity_node(Entity entity)
	{
		auto& tag = entity.get_component<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((selection_context== entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			selection_context = entity;
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

	}

	void SceneHierarchyPanel::draw_components(Entity entity)
	{
		if (entity.has_component<TagComponent>())
		{
			auto& tag = entity.get_component<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.has_component<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.get_component<TransformComponent>().Transform;
				ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);

				ImGui::TreePop();
			}
		}
	}

}