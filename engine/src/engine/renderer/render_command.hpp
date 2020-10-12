#pragma once

#include <glm/glm.hpp>

namespace fightingengine {

	class RenderCommand
	{
	public:
		static void init();

		static void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		static void set_clear_colour(const glm::vec4& color);

		static void clear();
	};
}
