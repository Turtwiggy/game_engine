
// //header
// #include "panels/scene_hierarchy_panel.hpp"

// //other library headers
// #include <imgui.h>
// #include <imgui_internal.h>
// #include <glm/gtc/type_ptr.hpp>

// //your project headers
// #include "engine/scene/components.hpp"
// using namespace fightingengine;

// namespace game_3d {

// 	SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene>& context)
// 	{
// 		set_context(context);
// 	}

// 	void SceneHierarchyPanel::set_context(const std::shared_ptr<Scene>& c)
// 	{
// 		context = c;
// 	}

// 	void SceneHierarchyPanel::on_imgui_render()
// 	{
// 		ImGui::Begin("Scene Hierarchy");

// 		context->get_registry().each([&](auto entityID)
// 		{
// 			Entity entity{ entityID , context.get() };
// 			draw_entity_node(entity);
// 		});

// 		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
// 			selection_context = {};

// 		ImGui::End();

// 		ImGui::Begin("Properties");
// 		if (selection_context)
// 			draw_components(selection_context);

// 		ImGui::End();
// 	}

// 	void SceneHierarchyPanel::draw_entity_node(Entity entity)
// 	{
// 		auto& tag = entity.get_component<TagComponent>().Tag;
		
// 		ImGuiTreeNodeFlags flags = ((selection_context== entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
// 		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
// 		if (ImGui::IsItemClicked())
// 		{
// 			selection_context = entity;
// 		}

// 		if (opened)
// 		{
// 			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
// 			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
// 			if (opened)
// 				ImGui::TreePop();
// 			ImGui::TreePop();
// 		}

// 	}

// 	template<typename T, typename UIFunction>
// 	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
// 	{
// 		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
// 		if (entity.has_component<T>())
// 		{
// 			auto& component = entity.get_component<T>();
// 			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

// 			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
// 			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
// 			ImGui::Separator();
// 			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
// 			ImGui::PopStyleVar(
// 			);
// 			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
// 			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
// 			{
// 				ImGui::OpenPopup("ComponentSettings");
// 			}

// 			bool removeComponent = false;
// 			if (ImGui::BeginPopup("ComponentSettings"))
// 			{
// 				if (ImGui::MenuItem("Remove component"))
// 					removeComponent = true;

// 				ImGui::EndPopup();
// 			}

// 			if (open)
// 			{
// 				uiFunction(component);
// 				ImGui::TreePop();
// 			}

// 			if (removeComponent)
// 				entity.remove_component<T>();
// 		}
// 	}

// 	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
// 	{
// 		ImGuiIO& io = ImGui::GetIO();
// 		auto boldFont = io.Fonts->Fonts[0];

// 		ImGui::PushID(label.c_str());

// 		ImGui::Columns(2);
// 		ImGui::SetColumnWidth(0, columnWidth);
// 		ImGui::Text(label.c_str());
// 		ImGui::NextColumn();

// 		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
// 		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

// 		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
// 		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

// 		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
// 		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
// 		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
// 		ImGui::PushFont(boldFont);
// 		if (ImGui::Button("X", buttonSize))
// 			values.x = resetValue;
// 		ImGui::PopFont();
// 		ImGui::PopStyleColor(3);

// 		ImGui::SameLine();
// 		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
// 		ImGui::PopItemWidth();
// 		ImGui::SameLine();

// 		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
// 		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
// 		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
// 		ImGui::PushFont(boldFont);
// 		if (ImGui::Button("Y", buttonSize))
// 			values.y = resetValue;
// 		ImGui::PopFont();
// 		ImGui::PopStyleColor(3);

// 		ImGui::SameLine();
// 		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
// 		ImGui::PopItemWidth();
// 		ImGui::SameLine();

// 		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
// 		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
// 		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
// 		ImGui::PushFont(boldFont);
// 		if (ImGui::Button("Z", buttonSize))
// 			values.z = resetValue;
// 		ImGui::PopFont();
// 		ImGui::PopStyleColor(3);

// 		ImGui::SameLine();
// 		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
// 		ImGui::PopItemWidth();

// 		ImGui::PopStyleVar();

// 		ImGui::Columns(1);

// 		ImGui::PopID();
// 	}

// 	void SceneHierarchyPanel::draw_components(Entity entity)
// 	{
// 		if (entity.has_component<TagComponent>())
// 		{
// 			auto& tag = entity.get_component<TagComponent>().Tag;

// 			char buffer[256];
// 			memset(buffer, 0, sizeof(buffer));
// 			strcpy_s(buffer, sizeof(buffer), tag.c_str());
// 			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
// 			{
// 				tag = std::string(buffer);
// 			}
// 		}

// 		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
// 		{
// 			DrawVec3Control("Translation", component.translation);
// 			glm::vec3 rotation = glm::degrees(component.rotation);
// 			DrawVec3Control("Rotation", rotation);
// 			component.rotation = glm::radians(rotation);
// 			DrawVec3Control("Scale", component.scale, 1.0f);
// 		});
// 	}


// } //namespace game_3d