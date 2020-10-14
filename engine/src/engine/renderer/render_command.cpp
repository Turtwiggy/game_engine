
//header
#include "engine/renderer/render_command.hpp"

//other library headers
#include <SDL2/SDL.h>
#include <GL/glew.h>

namespace fightingengine {

void RenderCommand::init()
{
    printf("init opengl \n");

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

    //Enable Multi Sampling
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Enable depth testing
    glEnable(GL_DEPTH_TEST);

    //From now on your rendered images will be gamma corrected and as this is done by the hardware it is completely free.
    //Something you should keep in mind with this approach (and the other approach) is that gamma correction (also) transforms the colors from linear space to non-linear space so it is very important you only do gamma correction at the last and final step. 
    //If you gamma-correct your colors before the final output, all subsequent operations on those colors will operate on incorrect values. 
    //For instance, if you use multiple framebuffers you probably want intermediate results passed in between framebuffers to remain in linear-space and only have the last framebuffer apply gamma correction before being sent to the monitor.
    //glEnable(GL_FRAMEBUFFER_SRGB);

    //Enable Faceculling
    // glEnable(GL_CULL_FACE);
    // glDepthFunc(GL_LESS);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void RenderCommand::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    printf("set viewport \n");
    glViewport(x, y, width, height);
}

void RenderCommand::set_clear_colour(const glm::vec4& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void RenderCommand::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

} //namespace fightingengine
