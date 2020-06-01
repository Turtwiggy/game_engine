#include "renderer.h"

#include "window/game_window.h"
#include "graphics/shapes.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace fightinggame;
using namespace fightinggame::graphics;

namespace fightinggame
{
    renderer::renderer(const game_window* window, bool vsync)
    {
        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // build and compile our shader program
        // ------------------------------------
        //flatColorShader = std::make_unique<Shader>("src/graphics/shaders/texture_transform.vs", "src/graphics/shaders/texture_transform.fs");

        // uncomment this call to draw in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        //texId = loadTexture("res/textures/Misc/orangecurvedwallsofsandstone.jpg");
        //flatColorShader->use();
        //flatColorShader->setInt("u_Texture", texId);
    }

    renderer::~renderer()
    {
        //_shaderManager.reset();
    }

    void renderer::configure_view(graphics::render_pass view_id, uint16_t width, uint16_t height) const
    {

    }

    void renderer::draw_scene(const draw_scene_desc& desc) const
    {
        //// Reflection Pass
        //{
        //	auto section = drawDesc.profiler.BeginScoped(Profiler::Stage::ReflectionPass);
        //	if (drawDesc.drawWater)
        //	{
        //		DrawSceneDesc drawPassDesc = drawDesc;

        //		auto& frameBuffer = drawDesc.water.GetFrameBuffer();
        //		auto reflectionCamera = drawDesc.camera->Reflect(drawDesc.water.GetReflectionPlane());

        //		drawPassDesc.viewId = graphics::RenderPass::Reflection;
        //		drawPassDesc.camera = reflectionCamera.get();
        //		drawPassDesc.frameBuffer = &frameBuffer;
        //		drawPassDesc.drawWater = false;
        //		drawPassDesc.drawDebugCross = false;
        //		drawPassDesc.drawBoundingBoxes = false;
        //		drawPassDesc.cullBack = true;
        //		DrawPass(meshPack, drawPassDesc);
        //	}
        //}

        //// Main Draw Pass
        //{
        //	auto section = drawDesc.profiler.BeginScoped(Profiler::Stage::MainPass);
        //	DrawPass(meshPack, drawDesc);
        //}

        draw_pass(desc);
    }

    void renderer::draw_pass(const draw_scene_desc& desc) const
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //bind texures
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texId);

        // activate shader
        //flatColorShader->use();

        // pass transformation matrices to the shader
        //flatColorShader->setMat4("u_ViewProjection", view_projection);

        //the end
    }

    void renderer::frame(SDL_Window* window)
    {
        SDL_GL_SwapWindow(window);
    }

    // utility function for loading a 2D texture from file
    // ---------------------------------------------------
    unsigned int renderer::loadTexture(char const* path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

}