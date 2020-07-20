
#pragma once

#include "graphics/vertex_buffer.h"

#include <memory>
#include <vector>

namespace fightinggame {

	class vertex_array
	{
	public:
		virtual ~vertex_array() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const std::shared_ptr<vertex_buffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<index_buffer>& indexBuffer) = 0;

		virtual const std::vector<std::shared_ptr<vertex_buffer>>& GetVertexBuffers() const = 0;
		virtual const std::shared_ptr<index_buffer>& GetIndexBuffer() const = 0;

		static std::shared_ptr<vertex_array> Create();
	};
}
