#pragma once

//other library headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//your project headers
#include "engine/3d/camera/camera.hpp"

namespace fightingengine {

struct TagComponent
{
	std::string Tag;

	TagComponent() = default;
	TagComponent(const TagComponent&) = default;
	TagComponent(const std::string& tag);
};

struct TransformComponent
{
	//glm::mat4 Transform{ 1.0f };
	glm::vec3 translation = {0.0f, 0.0f, 0.0f};
	glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
	glm::vec3 scale = {1.0f, 1.0f, 1.0f};

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3& t);

	glm::mat4 get_transform() const;
};

struct SpriteRendererComponent
{
	glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

	SpriteRendererComponent() = default;
	SpriteRendererComponent(const SpriteRendererComponent&) = default;
	SpriteRendererComponent(const glm::vec4& color)
		: Color(color) {}
};

struct CameraComponent
{
	Camera cam;
	bool Primary = false;
	bool FixedAspectRatio = false;

	CameraComponent() = default;
	CameraComponent(const CameraComponent&) = default;
};

} //namespace fightingengine