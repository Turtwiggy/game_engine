#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace fightinggame::graphics
{

	enum class render_pass : uint8_t
	{
		Main,
		//Reflection,
		ImGui,
		//MeshViewer,

		_count
	};

	static constexpr std::array<std::string_view, static_cast<uint8_t>(render_pass::_count)> RenderPassNames{
		"Main Pass",
		//"Reflection Pass",
		"ImGui Pass",
		//"Mesh Viewer Pass",
	};

}