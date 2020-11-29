#pragma once

#include <stdio.h>

namespace fightingengine
{
class RendererPBR
{
public:
    RendererPBR();

};

} //fightingengine


    //textures
    // ResourceManager::load_texture( "assets/textures/pbr_metalgrid/metalgrid2_basecolor.png", "albedo" );
    // ResourceManager::load_texture( "assets/textures/pbr_metalgrid/metalgrid2_normal-dx.png", "normal" );
    // ResourceManager::load_texture( "assets/textures/pbr_metalgrid/metalgrid2_metallic.png", "metallic" );
    // ResourceManager::load_texture( "assets/textures/pbr_metalgrid/metalgrid2_roughness.png", "roughness" );
    // ResourceManager::load_texture( "assets/textures/pbr_metalgrid/metalgrid2_AO.png", "ao" );

    // glm::vec3 light_positions[] = {
    //     glm::vec3(-10.0f,  10.0f, 10.0f),
    //     glm::vec3( 10.0f,  10.0f, 10.0f),
    //     glm::vec3(-10.0f, -10.0f, 10.0f),
    //     glm::vec3( 10.0f, -10.0f, 10.0f),
    // };
    // glm::vec3 light_colours[] = {
    //     glm::vec3(300.0f, 300.0f, 300.0f),
    //     glm::vec3(300.0f, 300.0f, 300.0f),
    //     glm::vec3(300.0f, 300.0f, 300.0f),
    //     glm::vec3(300.0f, 300.0f, 300.0f)
    // };
    // int nrRows = 7;
    // int nrColumns = 7;
    // float spacing = 2.5f;
    
    // pbr_shader_.bind();
    // pbr_shader_.set_mat4("view_projection", view_projection);  
    // pbr_shader_.set_vec3("camPos", camera.Position);  
    // pbr_shader_.set_vec3("albedo", 0.5f, 0.0f, 0.0f);
    // pbr_shader_.set_float("ao", 1.0f);

    // // Material	                F0 (Linear)	            F0 (sRGB)
    // // Water	                (0.02, 0.02, 0.02)	    (0.15, 0.15, 0.15)  	
    // // Plastic / Glass (Low)	(0.03, 0.03, 0.03)	    (0.21, 0.21, 0.21)	
    // // Plastic High	            (0.05, 0.05, 0.05)	    (0.24, 0.24, 0.24)	
    // // Glass (high) / Ruby	    (0.08, 0.08, 0.08)	    (0.31, 0.31, 0.31)	
    // // Diamond	                (0.17, 0.17, 0.17)	    (0.45, 0.45, 0.45)	
    // // Iron	                    (0.56, 0.57, 0.58)	    (0.77, 0.78, 0.78)	
    // // Copper	                (0.95, 0.64, 0.54)  	(0.98, 0.82, 0.76)	
    // // Gold	                    (1.00, 0.71, 0.29)  	(1.00, 0.86, 0.57)	
    // // Aluminium	            (0.91, 0.92, 0.92)  	(0.96, 0.96, 0.97)	
    // // Silver	                (0.95, 0.93, 0.88)  	(0.98, 0.97, 0.95

    // // ResourceManager::get_texture("albedo")->Bind(0);
    // // ResourceManager::get_texture("normal")->Bind(1);
    // // ResourceManager::get_texture("metallic")->Bind(2);
    // // ResourceManager::get_texture("roughness")->Bind(3);
    // // ResourceManager::get_texture("ao")->Bind(4);

    // // render rows*column number of spheres with material properties 
    // // defined by textures (they all have the same material properties)
    // glm::mat4 model = glm::mat4(1.0f);
    // for (int row = 0; row < nrRows; ++row)
    // {
    //     pbr_shader_.set_float("metallic", (float)row / (float)nrRows);

    //     for (int col = 0; col < nrColumns; ++col)
    //     {
    //         // we clamp the roughness to 0.025 - 1.0 as perfectly smooth
    //         // surfaces (roughness of 0.0) tend to look a bit off
    //         // on direct lighting.
    //         pbr_shader_.set_float("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

    //         model = glm::mat4(1.0f);
    //         model = glm::translate(model, glm::vec3(
    //             (float)(col - (nrColumns / 2)) * spacing,
    //             (float)(row - (nrRows / 2)) * spacing,
    //             0.0f
    //         ));
    //         pbr_shader_.set_mat4("model", model);

    //         renderSphere();
    //     }
    // }

    // // render light source (simply re-render sphere at light positions)
    // // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
    // // keeps the codeprint small.
    // float time_since_launch = app.get_time_since_launch();
    // for (unsigned int i = 0; i < sizeof(light_positions) / sizeof(light_positions[0]); ++i)
    // {
    //     glm::vec3 newPos = light_positions[i] + glm::vec3(sin(time_since_launch * 5.0) * 5.0, 0.0, 0.0);
    //     newPos = light_positions[i];
    //     pbr_shader_.set_vec3("lightPositions[" + std::to_string(i) + "]", newPos);
    //     pbr_shader_.set_vec3("lightColors[" + std::to_string(i) + "]", light_colours[i]);

    //     model = glm::mat4(1.0f);
    //     model = glm::translate(model, newPos);
    //     model = glm::scale(model, glm::vec3(0.5f));
    //     pbr_shader_.set_mat4("model", model);
    //     renderSphere();
    // }