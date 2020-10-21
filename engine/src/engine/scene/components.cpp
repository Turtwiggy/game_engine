
//header
#include "engine/scene/components.hpp"

namespace fightingengine{

TagComponent::TagComponent(const std::string& tag)
    : Tag(tag)
{
    
}

TransformComponent::TransformComponent(const glm::vec3& t)
    : translation(t)
{
}

glm::mat4 TransformComponent::get_transform() const
{
    glm::mat4 rot = 
          glm::rotate(glm::mat4(1.0f), rotation.x, { 1, 0, 0 })
        * glm::rotate(glm::mat4(1.0f), rotation.y, { 0, 1, 0 })
        * glm::rotate(glm::mat4(1.0f), rotation.z, { 0, 0, 1 });

    return glm::translate(glm::mat4(1.0f), translation)
        * rot
        * glm::scale(glm::mat4(1.0f), scale);
}

} //namespace fightingengine