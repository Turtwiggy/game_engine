#pragma once

//c++ standard lib headers
#include <vector>

//your project headers
#include "engine/mesh/mesh.hpp"
#include "engine/graphics/material.hpp"

namespace fightingengine {

class SceneNode;

class Scene 
{
public:
    static SceneNode* Root;
    static unsigned int CounterID;
public:
    //clears all the scene nodes
    static void clear();

    // static helper function that directly builds an empty scene node. Other sub-engines can 
    // directly add children to this empty scene node (w/ identity matrix as transform).
    static SceneNode* make_scene_node();
    //  similar to the MakeScene function, but directly builds a node w/ attached mesh and 
    // material.
    static SceneNode* make_scene_node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
    // because the scene manages we have to copy the nodes from here.
    static SceneNode* make_scene_node(SceneNode* node);

    // deletes a scene node from the global scene hierarchy (together with its  children).
    static void delete_scene_node(SceneNode* node);
};

} //namespace fightingengine