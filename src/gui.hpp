/*****************************************************************************
 * Copyright (c) 2018-2020 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "graphics/render_pass.h"
#include "graphics/renderer.h"

#include <imgui.h>

#include <array>
#include <memory>

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Cursor SDL_Cursor;
union SDL_Event;

namespace fightinggame
{
	//class Console;
	class game;
	class game_window;
	//class MeshViewer;
	class renderer;

	class Gui
	{
	public:
		Gui();
		virtual ~Gui();

		bool ProcessEventSdl2(const SDL_Event& event, ImGuiContext* imgui);
		void NewFrame(ImGuiContext* imgui);
		bool Loop(game& game, ImGuiContext* imgui);

	private:

		static const char* StaticGetClipboardText(void* ud) { return reinterpret_cast<Gui*>(ud)->GetClipboardText(); }
		static void StaticSetClipboardText(void* ud, const char* text) { reinterpret_cast<Gui*>(ud)->SetClipboardText(text); }
		const char* GetClipboardText();
		void SetClipboardText(const char* text);

		template <typename T, uint8_t N>
		struct CircularBuffer
		{
			static constexpr uint8_t _bufferSize = N;
			T _values[_bufferSize] = {};
			uint8_t _offset = 0;

			[[nodiscard]] T back() const { return _values[_offset]; }
			void pushBack(T value)
			{
				_values[_offset] = value;
				_offset = (_offset + 1u) % _bufferSize;
			}
		};

		uint64_t _time;
		CircularBuffer<float, 100> _times;
		CircularBuffer<float, 100> _fps;

		SDL_Window* _window;
		std::array<bool, 3> _mousePressed;
		std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> _mouseCursors;
		char* _clipboardTextData;
		int64_t _last;
		int32_t _lastScroll;
	};
} // namespace fightinggame
