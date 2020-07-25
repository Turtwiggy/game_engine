#pragma once

#include "graphics/renderer.h"
#include "window/game_window.h"
#include "util/profiler.hpp"
#include "util/circular_buffer.h"

#include <imgui.h>

#include <array>
#include <memory>

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Cursor SDL_Cursor;
union SDL_Event;

namespace fightinggame
{
	class Game;
	//class Console;
	//class game_window;
	//class MeshViewer;
	//class renderer;

	class Gui
	{
	public:
		Gui();

		bool ProcessEventSdl2(const SDL_Event& event, ImGuiContext* imgui);
		bool Loop(Game& game, Profiler& profiler);

	private:

		static const char* StaticGetClipboardText(void* ud) { return reinterpret_cast<Gui*>(ud)->GetClipboardText(); }
		static void StaticSetClipboardText(void* ud, const char* text) { reinterpret_cast<Gui*>(ud)->SetClipboardText(text); }
		const char* GetClipboardText();
		void SetClipboardText(const char* text);

		void ShowProfilerWindow(Game& game, Profiler& profiler);

		SDL_Window* _window;
		std::array<bool, 3> _mousePressed;
		std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> _mouseCursors;
		char* _clipboardTextData;
		int64_t _last;
		int32_t _lastScroll;
	};
}
