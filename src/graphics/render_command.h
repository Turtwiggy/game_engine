#pragma once

#include "graphics/renderer_api.h"

namespace fightinggame {

	class RenderCommand
	{
	public:
		static void init()
		{
			s_RendererAPI->init();
		}

		static void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->set_viewport(x, y, width, height);
		}

		static void set_clear_colour(const glm::vec4& color)
		{
			s_RendererAPI->set_clear_colour(color);
		}

		static void clear()
		{
			s_RendererAPI->clear();
		}

		static void draw_indexed(const std::shared_ptr<vertex_array>& vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->draw_indexed(vertexArray, count);
		}
	private:
		static std::unique_ptr<renderer_api> s_RendererAPI;
	};
}
