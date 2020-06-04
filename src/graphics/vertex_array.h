
#pragma once

#include "base.h"
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

		virtual void AddVertexBuffer(const Ref<vertex_buffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<index_buffer>& indexBuffer) = 0;

		virtual const std::vector<Ref<vertex_buffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<index_buffer>& GetIndexBuffer() const = 0;

		static Ref<vertex_array> Create();
	};

}