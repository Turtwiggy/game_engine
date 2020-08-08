#include "renderer_pbr.h"

namespace fightinggame {

    struct PBRData {

        //input for debugging
        bool is_c_held = false;

        uint32_t draw_calls = 0;
    };
    static PBRData s_Data;

    void RendererPBR::init(int screen_width, int screen_height)
    {
        printf("hello init");

        // A quad shader to render the full-screen quad VAO with the framebuffer as texture
        // --------------------------------------------------------------------------------
        //Shader quad_shader = Shader()
        //    .attach_shader("assets/shaders/raytraced/example.vert", GL_VERTEX_SHADER)
        //    .attach_shader("assets/shaders/raytraced/example.frag", GL_FRAGMENT_SHADER)
        //    .build_program();
        //quad_shader.use();
        //quad_shader.setInt("tex", 0);
        //s_Data.quad_shader = quad_shader;
        //quad_shader.unbind();
    }

    void RendererPBR::draw_pass(draw_scene_desc& desc, const GameState& state)
    {
        int width, height = 0;
        GameWindow& window = desc.window;
        window.GetSize(width, height);
        glm::mat4 view_projection = desc.camera.get_view_projection_matrix(width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Render a cube
        //{
        //    s_Data.geometry_shader.use();
        //    s_Data.geometry_shader.setMat4("view_projection", view_projection);

        //    //either this or textures
        //    //s_Data.geometry_shader.setVec3("diffuse", glm::vec3(1.0f, 0.0f, 0.0f));
        //    s_Data.geometry_shader.setFloat("specular", 1.0f);

        //    glm::mat4 model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
        //    model = glm::scale(model, glm::vec3(1.0f));
        //    s_Data.geometry_shader.setMat4("model", model);
        //    state.cornel_box->model->draw(s_Data.geometry_shader, s_Data.draw_calls);
        //}

        //Cornell
        //{
        //    s_Data.object_shader.use();
        //    s_Data.object_shader.setMat4("view_projection", view_projection);
        //    //cube's material properties
        //    s_Data.object_shader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        //    s_Data.object_shader.setFloat("material.shininess", 32.0f);
        //    //flat lighting
        //    glm::vec3 light_direction = glm::vec3(-0.2, -1.0, -0.3);
        //    s_Data.object_shader.setVec3("light.direction", light_direction);
        //    s_Data.object_shader.setVec3("light.ambient", glm::vec3(0.2, 0.2, 0.2));
        //    s_Data.object_shader.setVec3("light.diffuse", glm::vec3(0.5, 0.5, 0.5)); // darken diffuse light a bit
        //    s_Data.object_shader.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));

        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
        //    model = glm::scale(model, glm::vec3(0.5f));
        //    s_Data.object_shader.setMat4("model", model);
        //    state.cornel_box->model->draw(s_Data.object_shader, s_Data.stats.DrawCalls);
        //}

        ImGui::Begin("RayRenderer Profiler");
        ImGui::Text("Draw Calls: %i", s_Data.draw_calls);
        ImGui::End();
        s_Data.is_c_held = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_C));

        s_Data.draw_calls = 0;
    }

    void RendererPBR::resize(int width, int height)
    {
        printf("hello resize");
    }

    
}
