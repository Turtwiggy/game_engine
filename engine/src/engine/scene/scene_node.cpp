
//header
#include "engine/scene/scene_node.hpp"

//other project headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//your project headers
#include "engine/scene/scene.hpp"
#include "engine/mesh/mesh.hpp"
#include "engine/graphics/material.hpp"

namespace fightingengine {

   
SceneNode::SceneNode(unsigned int id) : id_(id)
{
    
}

SceneNode::~SceneNode()
{
    // traverse the list of children and delete accordingly.
    for (unsigned int i = 0; i < children_.size(); ++i)
    {
        // it should not be possible that a scene node is childed by more than one
        // parent, thus we don't need to care about deleting dangling pointers.
        delete children_[i];
    }
}

void SceneNode::set_position(glm::vec3 position)
{
    position_ = position;
    dirty_ = true;
}

void SceneNode::set_rotation(glm::vec4 rotation)
{
    rotation_ = rotation;
    dirty_ = true;
}

void SceneNode::set_scale(glm::vec3 scale)
{
    scale_ = scale;
    dirty_ = true;
}

void SceneNode::set_scale(float scale)
{
    scale_ = glm::vec3(scale);
    dirty_ = true;
}

glm::vec3 SceneNode::get_local_position() const
{
    return position_;
}

glm::vec4 SceneNode::get_local_rotation() const
{
    return rotation_;
}

glm::vec3 SceneNode::get_local_scale() const
{
    return scale_;
}

glm::vec3 SceneNode::get_world_position()
{
    glm::mat4 transform = get_transform();
    glm::vec4 pos = transform * glm::vec4(position_, 1.0f);
    return glm::vec3(pos.x, pos.y, pos.z);
}

glm::vec3 SceneNode::get_world_scale()
{
    glm::mat4 transform = get_transform();
    glm::vec3 scale = glm::vec3(transform[0][0], transform[1][1], transform[2][2]);
    if (scale.x < 0.0f) scale.x *= -1.0f;
    if (scale.y < 0.0f) scale.y *= -1.0f;
    if(scale.z < 0.0f) scale.z *= -1.0f;
    return scale;
}

unsigned int SceneNode::get_id() const
{
    return id_;
}

void SceneNode::add_child(SceneNode *node)
{
    // check if this child already has a parent. If so, first remove this scene node from its 
    // current parent. Scene nodes aren't allowed to exist under multiple parents.
    if (node->parent_)
    {
        node->parent_->remove_child(node->id_);
    }
    node->parent_ = this;
    children_.push_back(node);
}

void SceneNode::remove_child(unsigned int id)
{
    auto it = std::find(children_.begin(), children_.end(), get_child(id));
    if(it != children_.end())
        children_.erase(it);
}

std::vector<SceneNode*> SceneNode::get_children() const
{
    return children_;
}

unsigned int SceneNode::get_child_count() const
{
    return children_.size();
}

SceneNode *SceneNode::get_child(unsigned int id) const
{
    for (unsigned int i = 0; i < children_.size(); ++i)
    {
        if(children_[i]->get_id() == id)
            return children_[i];
    }
    return nullptr;
}

SceneNode* SceneNode::get_child_by_index(unsigned int index) const
{
    assert(index < get_child_count());
    return children_[index];
}

SceneNode *SceneNode::get_parent() const
{
    return parent_;
}

glm::mat4 SceneNode::get_transform()
{
    if (dirty_)
    {
        update_transform(false);
    }
    return transform_;
}

glm::mat4 SceneNode::get_prev_transform() const
{
    return prev_transform_;
}

void SceneNode::update_transform(bool updatePrevTransform)
{
    // if specified, store current transform as prev transform (for calculating motion vectors)
    if (updatePrevTransform)
    {
        prev_transform_ = transform_;     
    }

    // we only do this if the node itself or its parent is flagged as dirty
    if (dirty_)
    {
        // first scale, then rotate, then translate
        const glm::mat4 model = glm::mat4(1.0f);
        transform_ = glm::translate(model, position_);
        transform_ = glm::scale(transform_, scale_);
        transform_ = glm::rotate(transform_, rotation_.w, glm::vec3(rotation_));
        
        if (parent_)
        {
            transform_ = parent_->transform_ * transform_;
        }        
    }
    for (int i = 0; i < children_.size(); ++i)
    {
        if (dirty_)
        {
            children_[i]->dirty_ = true;
        }
        children_[i]->update_transform(updatePrevTransform);
    }
    dirty_ = false;
}

} //namespace fightingengine