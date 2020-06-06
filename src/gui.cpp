/*****************************************************************************
 * Copyright (c) 2018-2020 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "gui.hpp"

#include "game.hpp"
#include "window/game_window.h"

//#include "LHVMViewer.h"
//#include "MeshViewer.h"
//#include "Profiler.h"

#include <imgui.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif

#include <GL/glew.h>
#include <examples/imgui_impl_sdl.h>

using namespace fightinggame;

#define IMGUI_FLAGS_NONE UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

Gui::Gui( )
	: _time(0)
	, _mousePressed{ false, false, false }
	, _mouseCursors{ 0 }
	, _clipboardTextData(nullptr)
	, _lastScroll(0)
{
}

bool Gui::ProcessEventSdl2(const SDL_Event& event, ImGuiContext* imgui)
{
	ImGui::SetCurrentContext(imgui);

	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
	case SDL_QUIT:
		return false;
	case SDL_MOUSEWHEEL:
	{
		if (event.wheel.x > 0)
			io.MouseWheelH += 1;
		if (event.wheel.x < 0)
			io.MouseWheelH -= 1;
		if (event.wheel.y > 0)
			io.MouseWheel += 1;
		if (event.wheel.y < 0)
			io.MouseWheel -= 1;
		return io.WantCaptureMouse;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		if (event.button.button == SDL_BUTTON_LEFT)
			_mousePressed[0] = true;
		if (event.button.button == SDL_BUTTON_RIGHT)
			_mousePressed[1] = true;
		if (event.button.button == SDL_BUTTON_MIDDLE)
			_mousePressed[2] = true;
		return io.WantCaptureMouse;
	}
	case SDL_TEXTINPUT:
	{
		io.AddInputCharactersUTF8(event.text.text);
		return io.WantTextInput;
	}
	case SDL_KEYDOWN:
		//if (event.key.keysym.sym == SDLK_BACKQUOTE)
		//{
		//}
	case SDL_KEYUP:
	{
		int key = event.key.keysym.scancode;
		IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
		io.KeysDown[key] = (event.type == SDL_KEYDOWN);
		io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
		io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
		return io.WantCaptureKeyboard;
	}
	}
	return io.WantCaptureMouse;
}

const char* Gui::GetClipboardText()
{
	if (_clipboardTextData)
		SDL_free(_clipboardTextData);
	_clipboardTextData = SDL_GetClipboardText();
	return _clipboardTextData;
}

void Gui::SetClipboardText(const char* text)
{
	SDL_SetClipboardText(text);
}

bool Gui::Loop(game& game, ImGuiContext* imgui)
{
	//printf("in gui loop");

	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 8.0f, io.DisplaySize.y - 8.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
	ImGui::SetNextWindowBgAlpha(0.35f);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Debug"))
		{
			if (ImGui::MenuItem("Quit", "Esc"))
			{
				return true;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (true) //move to config variable
		ShowProfilerWindow();

	ImGui::Render();

	return false;
}

void Gui::ShowProfilerWindow()
{
	ImGui::Begin("Profiler");
	if (ImGui::Button("Hello Profiler"))
	{
		printf("you clicked a button");
	};
	ImGui::End();
}