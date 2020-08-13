
#pragma once

#include "graphics/renderer_api.h"

#include <memory>

namespace fightingengine {

	class opengl_renderer_api : public RendererAPI
	{
	public:
		virtual void init() override;
		virtual void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void set_clear_colour(const glm::vec4& color) override;
		virtual void clear() override;
    };
}
