
#include "graphics/opengl/opengl_renderer_api.h"

#include <gl/glew.h>

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

        //Can put OpenGL Specific commands here
        //e.g. glEnable(GL_DEPTH_TEST);
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

	void opengl_renderer_api::draw_indexed(const std::shared_ptr<vertex_array>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

}
