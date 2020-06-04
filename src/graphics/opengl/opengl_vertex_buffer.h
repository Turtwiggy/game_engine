#pragma once

#include "graphics/vertex_buffer.h"

namespace fightinggame {

	class opengl_vertex_buffer : public vertex_buffer
	{
	public:
		opengl_vertex_buffer(uint32_t size);
		opengl_vertex_buffer(float* vertices, uint32_t size);
		virtual ~opengl_vertex_buffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, uint32_t size) override;

		virtual const buffer_layout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const buffer_layout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		buffer_layout m_Layout;
	};

	class opengl_index_buffer : public index_buffer
	{
	public:
		opengl_index_buffer(uint32_t* indices, uint32_t count);
		virtual ~opengl_index_buffer();

		virtual void Bind() const;
		virtual void Unbind() const;

		virtual uint32_t GetCount() const { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

}