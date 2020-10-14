
//header
#include "engine/scene/scene.hpp"

//other library headers
#include <glm/glm.hpp>

//your project headers
#include "engine/scene/components.hpp"
#include "engine/scene/entity.hpp"

namespace fightingengine {

Scene::Scene()
{
}

Scene::~Scene()
{
}

Entity Scene::create_entity(const std::string& name)
{
	Entity entity = { registry.create(), this };

	entity.add_component<TransformComponent>();

	auto& tag = entity.add_component<TagComponent>();
	tag.Tag = name.empty() ? "Entity" : name;

	return entity;
}

void Scene::on_update(const float delta_time)
{

	//How to render scene?
	//printf("work me out!");

	Camera* main_camera = nullptr;
	glm::mat4* camera_transform = nullptr;
	{
		auto view = registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view)
		{
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
			
			if (camera.Primary)
			{
				main_camera = &camera.cam;
				camera_transform = &transform.Transform;
				break;
			}
		}
	}

	if(main_camera)
	{
		// Renderer2D::BeginScene(*mainCamera, *cameraTransform);
		// auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		// for (auto entity : group)
		// {
		// 	auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
		// 	Renderer2D::DrawQuad(transform, sprite.Color);
		// }
		// Renderer2D::EndScene();
	}
}

void Scene::on_viewport_resize(uint32_t width, uint32_t height)
{
	viewport_width = width;
	viewport_height = height;

	// Resize our non-FixedAspectRatio cameras
	auto view = registry.view<CameraComponent>();
	for (auto entity : view)
	{
		auto& camera_component = view.get<CameraComponent>(entity);
		//if (!camera_component.FixedAspectRatio)
			//camera_component.cam.(width, height);
	}
}

} //namespace fightingengine