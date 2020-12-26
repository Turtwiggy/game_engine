
// header
#include "engine/scene/scene_node.hpp"

// other project headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// your project headers
#include "engine/graphics/material.hpp"
#include "engine/mesh/mesh.hpp"

namespace fightingengine {

SceneNode::SceneNode(unsigned int id)
  : id_(id)
{}

SceneNode::~SceneNode()
{
  // traverse the list of children and delete accordingly.
  // for (unsigned int i = 0; i < children_.size(); ++i)
  // {
  //     // it should not be possible that a scene node is childed by more than
  //     one
  //     // parent, thus we don't need to care about deleting dangling pointers.
  //     delete children_[i];
  // }
}

void
SceneNode::set_position(glm::vec3 position)
{
  position_ = position;
  dirty_ = true;
}

void
SceneNode::set_rotation(glm::vec4 rotation)
{
  rotation_ = rotation;
  dirty_ = true;
}

void
SceneNode::set_scale(glm::vec3 scale)
{
  scale_ = scale;
  dirty_ = true;
}

void
SceneNode::set_scale(float scale)
{
  scale_ = glm::vec3(scale);
  dirty_ = true;
}

glm::vec3
SceneNode::get_local_position() const
{
  return position_;
}

glm::vec4
SceneNode::get_local_rotation() const
{
  return rotation_;
}

glm::vec3
SceneNode::get_local_scale() const
{
  return scale_;
}

glm::vec3
SceneNode::get_world_position()
{
  glm::mat4 transform = get_transform();
  glm::vec4 pos = transform * glm::vec4(position_, 1.0f);
  return glm::vec3(pos.x, pos.y, pos.z);
}

glm::vec3
SceneNode::get_world_scale()
{
  glm::mat4 transform = get_transform();
  glm::vec3 scale = glm::vec3(transform[0][0], transform[1][1], transform[2][2]);
  if (scale.x < 0.0f)
    scale.x *= -1.0f;
  if (scale.y < 0.0f)
    scale.y *= -1.0f;
  if (scale.z < 0.0f)
    scale.z *= -1.0f;
  return scale;
}

glm::mat4
SceneNode::get_transform()
{
  if (dirty_) {
    update_transform(false);
  }
  return transform_;
}

glm::mat4
SceneNode::get_prev_transform() const
{
  return prev_transform_;
}

void
SceneNode::update_transform(bool updatePrevTransform)
{
  // if specified, store current transform as prev transform (for calculating
  // motion vectors)
  if (updatePrevTransform) {
    prev_transform_ = transform_;
  }

  // we only do this if the node itself or its parent is flagged as dirty
  if (dirty_) {
    // first scale, then rotate, then translate
    const glm::mat4 model = glm::mat4(1.0f);
    transform_ = glm::translate(model, position_);
    transform_ = glm::scale(transform_, scale_);
    transform_ = glm::rotate(transform_, rotation_.w, glm::vec3(rotation_));
  }

  //     if (parent_)
  //     {
  //         transform_ = parent_->transform_ * transform_;
  //     }
  // }
  // for (int i = 0; i < children_.size(); ++i)
  // {
  //     if (dirty_)
  //     {
  //         children_[i]->dirty_ = true;
  //     }
  //     children_[i]->update_transform(updatePrevTransform);
  // }

  dirty_ = false;
}

} // namespace fightingengine