#pragma once

#include "graphics/renderer_api.h"

#include <memory>

namespace fightingengine {

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

	private:
		static std::unique_ptr<RendererAPI> s_RendererAPI;
	};
}
