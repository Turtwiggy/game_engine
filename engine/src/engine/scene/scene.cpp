
//header
#include "engine/scene/scene.hpp"

//c++ stl
#include <stack>
#include <memory>

//your project files
#include "engine/scene/scene_node.hpp"

namespace fightingengine
{
SceneNode* Scene::Root = new SceneNode(0);
unsigned int Scene::CounterID = 0;    

void Scene::clear()
{       
    Scene::delete_scene_node(Root);
    Scene::Root = new SceneNode(0);
}

SceneNode* Scene::make_scene_node()
{
    SceneNode* node = new SceneNode(Scene::CounterID++);
    // keep a global rerefence to this scene node s.t.we can clear the scene's nodes for memory 
    // management: end of program or when switching scenes.
    Root->add_child(node);
    return node;
}

SceneNode* Scene::make_scene_node(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
    {
        SceneNode* node = new SceneNode(Scene::CounterID++);

        node->Mesh = mesh;
        node->Material = material;

        // keep a global rerefence to this scene node s.t. we can clear the scene's nodes for 
        // memory management: end of program or when switching scenes.
        Root->add_child(node);
        return node;
    }

SceneNode* Scene::make_scene_node(SceneNode* node)
{
    SceneNode *newNode = new SceneNode(Scene::CounterID++);

    newNode->Mesh     = node->Mesh;
    newNode->Material = node->Material;
    newNode->BoxMin   = node->BoxMin;
    newNode->BoxMax   = node->BoxMax;

    // traverse through the list of children and add them correspondingly
    std::stack<SceneNode*> nodeStack;
    for (unsigned int i = 0; i < node->get_child_count(); ++i)
        nodeStack.push(node->get_child_by_index(i));
    while (!nodeStack.empty())
    {
        SceneNode* child = nodeStack.top();
        nodeStack.pop();
        // similarly, create SceneNode for each child and push to scene node memory list.
        SceneNode* newChild = new SceneNode(Scene::CounterID++);
        newChild->Mesh     = child->Mesh;
        newChild->Material = child->Material;
        newChild->BoxMin   = child->BoxMin;
        newChild->BoxMax   = child->BoxMax;
        newNode->add_child(newChild);

        for (unsigned int i = 0; i < child->get_child_count(); ++i)
            nodeStack.push(child->get_child_by_index(i));
    }

    Root->add_child(newNode);
    return newNode;
}

void Scene::delete_scene_node(SceneNode *node)
{
    if (node->get_parent())
    {
        node->get_parent()->remove_child(node->get_id());
    }
    // all delete logic is contained within each scene node's destructor.
    delete node;
}

} //namespace fightingengine