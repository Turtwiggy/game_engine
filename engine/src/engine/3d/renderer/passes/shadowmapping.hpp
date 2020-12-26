#pragma once

// other library headers
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

// your project headers
#include "engine/graphics/shader.hpp"

namespace fightingengine{

class ShadowmappingRenderpass
{
public:
    Shader shadowmap_depth_shader;
    unsigned int depthmap_tex = 0;

private:
    int shadowmap_width_ = 2048;
    int shadowmap_height_ = 2048;
    unsigned int depthmap_fbo_ = 0;

public:
    ShadowmappingRenderpass();  

    // initialize a shadowmap fbo
    // attach a shadowmap texture to the depth component
    void create_shadowmap_depthbuffer(    
        unsigned int& depth_map_fbo, 
        unsigned int& depth_map_tex, 
        int tex_width, 
        int tex_height );

    glm::mat4 calculate_light_space_matrix( 
        const glm::vec3& light_pos );

    // render the scene after this call
    void bind( const glm::mat4& light_space_matrix );
    void unbind();
};

} //namespace fightingengine