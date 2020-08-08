
#include "graphics/opengl/opengl_renderer_api.hpp"

#include <gl/glew.h>

#include <SDL2/SDL.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif

namespace fightinggame {

	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		//switch (severity)
		//{
		//case GL_DEBUG_SEVERITY_HIGH:         CORE_CRITICAL(message); return;
		//case GL_DEBUG_SEVERITY_MEDIUM:       CORE_ERROR(message); return;
		//case GL_DEBUG_SEVERITY_LOW:          CORE_WARN(message); return;
		//case GL_DEBUG_SEVERITY_NOTIFICATION: CORE_TRACE(message); return;
		//}
	}

	void opengl_renderer_api::init()
	{
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

        //Enable Faceculling
        //glEnable(GL_CULL_FACE);
        //glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Enable depth testing
        glEnable(GL_DEPTH_TEST);

        //From now on your rendered images will be gamma corrected and as this is done by the hardware it is completely free.
        //Something you should keep in mind with this approach (and the other approach) is that gamma correction (also) transforms the colors from linear space to non-linear space so it is very important you only do gamma correction at the last and final step. 
        //If you gamma-correct your colors before the final output, all subsequent operations on those colors will operate on incorrect values. 
        //For instance, if you use multiple framebuffers you probably want intermediate results passed in between framebuffers to remain in linear-space and only have the last framebuffer apply gamma correction before being sent to the monitor.
        glEnable(GL_FRAMEBUFFER_SRGB);

	}

	void opengl_renderer_api::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void opengl_renderer_api::set_clear_colour(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void opengl_renderer_api::clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
}
