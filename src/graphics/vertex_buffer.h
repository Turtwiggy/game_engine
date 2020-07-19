#pragma once

#include "util/base.h"

#include <cassert>
#include <string>
#include <vector>

namespace fightinggame {

	enum class shader_data_type
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(shader_data_type type)
	{
		switch (type)
		{
		case shader_data_type::Float:    return 4;
		case shader_data_type::Float2:   return 4 * 2;
		case shader_data_type::Float3:   return 4 * 3;
		case shader_data_type::Float4:   return 4 * 4;
		case shader_data_type::Mat3:     return 4 * 3 * 3;
		case shader_data_type::Mat4:     return 4 * 4 * 4;
		case shader_data_type::Int:      return 4;
		case shader_data_type::Int2:     return 4 * 2;
		case shader_data_type::Int3:     return 4 * 3;
		case shader_data_type::Int4:     return 4 * 4;
		case shader_data_type::Bool:     return 1;
		}

		assert(false); //"unknown shaderdatatype"
		return 0;
	}

	struct buffer_element
	{
		std::string Name;
		shader_data_type Type;
		uint32_t Size;
		size_t Offset;
		bool Normalized;

		buffer_element() = default;

		buffer_element(shader_data_type type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case shader_data_type::Float:   return 1;
			case shader_data_type::Float2:  return 2;
			case shader_data_type::Float3:  return 3;
			case shader_data_type::Float4:  return 4;
			case shader_data_type::Mat3:    return 3; // 3* float3
			case shader_data_type::Mat4:    return 4; // 4* float4
			case shader_data_type::Int:     return 1;
			case shader_data_type::Int2:    return 2;
			case shader_data_type::Int3:    return 3;
			case shader_data_type::Int4:    return 4;
			case shader_data_type::Bool:    return 1;
			}

			assert(false); //"unknown shaderdatatype"
			return 0;
		}
	};

	class buffer_layout
	{
	public:
		buffer_layout() {}

		buffer_layout(const std::initializer_list<buffer_element>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		uint32_t GetStride() const { return m_Stride; }
		const std::vector<buffer_element>& GetElements() const { return m_Elements; }

		std::vector<buffer_element>::iterator begin() { return m_Elements.begin(); }
		std::vector<buffer_element>::iterator end() { return m_Elements.end(); }
		std::vector<buffer_element>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<buffer_element>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<buffer_element> m_Elements;
		uint32_t m_Stride = 0;
	};

	class vertex_buffer
	{
	public:
		virtual ~vertex_buffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const buffer_layout& GetLayout() const = 0;
		virtual void SetLayout(const buffer_layout& layout) = 0;

		static Ref<vertex_buffer> Create(uint32_t size);
		static Ref<vertex_buffer> Create(float* vertices, uint32_t size);
	};

	// Currently only supports 32-bit index buffers
	class index_buffer
	{
	public:
		virtual ~index_buffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<index_buffer> Create(uint32_t* indices, uint32_t count);
	};
}
