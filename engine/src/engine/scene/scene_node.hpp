#pragma once

//c++ standard library
#include <vector>

//other library headers
#include <GL/glew.h>

//your project headers
#include "engine/scene/scene.hpp"
#include "engine/mesh/mesh.hpp"
#include "engine/graphics/material.hpp"

namespace fightingengine
{

    /*
      An individual scene node that links a mesh to a material
      to render the mesh with while at the same time
      maintaining a parent-child hierarchy for transform
      relations.
      Each node can have any number of children (via a linked
      list) and represents a single renderable entity in a
      larger scene where each child transform on top of their
      parent node.
    */
    class SceneNode
    {
    public:
        // each node contains relevant render state
        Mesh*     Mesh;
        Material* Material;

        // bounding box 
        glm::vec3 BoxMin = glm::vec3(-99999.0f);
        glm::vec3 BoxMax = glm::vec3( 99999.0f);
        
    private:
        std::vector<SceneNode*> children_;
        SceneNode *parent_;

        // per-node transform (w/ parent-child relationship)
        glm::mat4 transform_;
        glm::mat4 prev_transform_;
        glm::vec3 position_ = glm::vec3(0.0f);
        glm::vec4 rotation_; // axis-angle for now; test w/ quaternions soon!
        glm::vec3 scale_ = glm::vec3(1.0f);

        // mark the current node's tranform as dirty
        // if it needs to be re-calculated this frame
        bool dirty_;

        // each node is uniquely identified by
        // a 32-bit incrementing unsigned integer
        unsigned int id_;

        static unsigned int CounterID;
    public:
        SceneNode(unsigned int id);
        ~SceneNode();

        // scene node transform
        void set_position(glm::vec3 position);
        void set_rotation(glm::vec4 rotation);
        void set_scale(glm::vec3 scale);
        void set_scale(float scale);
        glm::vec3 get_local_position();
        glm::vec4 get_local_rotation();
        glm::vec3 get_local_scale();
        glm::vec3 get_world_position();
        glm::vec3 get_world_scale();

        // scene graph 
        unsigned int get_id();
        void add_child(SceneNode *node);
        void remove_child(unsigned int id);
        std::vector<SceneNode*> get_children();
        unsigned int            get_child_count();
        SceneNode              *get_child(unsigned int id);
        SceneNode              *get_child_by_index(unsigned int index);
        SceneNode              *get_parent();

        // returns the transform of the current node 
        // combined with its parent(s)' transform.
        glm::mat4 get_transform();
        glm::mat4 get_prev_transform();

        // re-calculates this node and its children's 
        // transform components if its parent or the 
        // node itself is dirty.
        void UpdateTransform(bool updatePrevTransform = false);
    };
}
