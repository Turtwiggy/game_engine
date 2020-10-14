#pragma once

//c++ standard library headers
#include <string>

//other library headers
#include <entt/entt.hpp>

//your project headers
//#include "engine/scene/entity.hpp"

namespace fightingengine {

class Entity;

class Scene 
{
public:
    Scene();
    ~Scene();

    Entity create_entity(const std::string& name = std::string());

    void on_update(const float delta_time);
    void on_viewport_resize(uint32_t width, uint32_t height);

    entt::registry& get_registry(){return registry;};

private:
    entt::registry registry;
    int32_t viewport_width = 0, viewport_height = 0;

    friend class Entity;
};

} //namespace fightingengine