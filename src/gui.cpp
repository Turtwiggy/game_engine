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

Gui::~Gui()
{
	ImGui::DestroyContext();
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


void Gui::NewFrame(ImGuiContext* imgui)
{
	ImGui::SetCurrentContext(imgui);

	ImGui::NewFrame();
}

bool Gui::Loop(game& game, ImGuiContext* imgui)
{
	NewFrame(imgui);

	//printf("in gui loop");

	ImGui::Begin("Hello World!");
	if (ImGui::Button("Hello button"))
	{
		printf("you clicked a button");
	};
	ImGui::End();

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

	ImGui::Render();

	return false;
}

//
//void Gui::ShowProfilerWindow(Game& game)
//{
//	if (ImGui::Begin("Profiler", &game.GetConfig().showProfiler))
//	{
//		const bgfx::Stats* stats = bgfx::getStats();
//		const double toMsCpu = 1000.0 / stats->cpuTimerFreq;
//		const double toMsGpu = 1000.0 / stats->gpuTimerFreq;
//		const double frameMs = double(stats->cpuTimeFrame) * toMsCpu;
//		_times.pushBack(frameMs);
//		_fps.pushBack(1000.0f / frameMs);
//
//		char frameTextOverlay[256];
//		std::snprintf(frameTextOverlay, sizeof(frameTextOverlay), "%.3fms, %.1f FPS", _times.back(), _fps.back());
//
//		ImGui::Text("Submit CPU %0.3f, GPU %0.3f (Max GPU Latency: %d)",
//			double(stats->cpuTimeEnd - stats->cpuTimeBegin) * toMsCpu,
//			double(stats->gpuTimeEnd - stats->gpuTimeBegin) * toMsGpu, stats->maxGpuLatency);
//		ImGui::Text("Wait Submit %0.3f, Wait Render %0.3f", stats->waitSubmit * toMsCpu, stats->waitRender * toMsCpu);
//
//		ImGui::Columns(5);
//		ImGui::Checkbox("Sky", &game.GetConfig().drawSky);
//		ImGui::NextColumn();
//		ImGui::Checkbox("Water", &game.GetConfig().drawWater);
//		ImGui::NextColumn();
//		ImGui::Checkbox("Island", &game.GetConfig().drawIsland);
//		ImGui::NextColumn();
//		ImGui::Checkbox("Entities", &game.GetConfig().drawEntities);
//		ImGui::NextColumn();
//		ImGui::Checkbox("TestModel", &game.GetConfig().drawTestModel);
//		ImGui::NextColumn();
//		ImGui::Checkbox("Debug Cross", &game.GetConfig().drawDebugCross);
//		ImGui::Columns(1);
//
//		auto width = ImGui::GetColumnWidth() - ImGui::CalcTextSize("Frame").x;
//		ImGui::PlotHistogram("Frame", _times._values, decltype(_times)::_bufferSize, _times._offset, frameTextOverlay, 0.0f,
//			FLT_MAX, ImVec2(width, 45.0f));
//
//		ImGui::Text("Primitives Triangles %u, Triangle Strips %u, Lines %u "
//			"Line Strips %u, Points %u",
//			stats->numPrims[0], stats->numPrims[1], stats->numPrims[2], stats->numPrims[3], stats->numPrims[4]);
//		ImGui::Columns(2);
//		ImGui::Text("Num Entities %u, Trees %u", static_cast<uint32_t>(game.GetEntityRegistry().Size<Transform>()),
//			static_cast<uint32_t>(game.GetEntityRegistry().Size<Tree>()));
//		ImGui::Text("Num Draw %u, Num Compute %u, Num Blit %u", stats->numDraw, stats->numCompute, stats->numBlit);
//		ImGui::Text("Num Buffers Index %u, Vertex %u", stats->numIndexBuffers, stats->numVertexBuffers);
//		ImGui::Text("Num Dynamic Buffers Index %u, Vertex %u", stats->numDynamicIndexBuffers, stats->numDynamicVertexBuffers);
//		ImGui::Text("Num Transient Buffers Index %u, Vertex %u", stats->transientIbUsed, stats->transientVbUsed);
//		ImGui::NextColumn();
//		ImGui::Text("Num Vertex Layouts %u", stats->numVertexLayouts);
//		ImGui::Text("Num Textures %u, FrameBuffers %u", stats->numTextures, stats->numFrameBuffers);
//		ImGui::Text("Memory Texture %ld, RenderTarget %ld", stats->textureMemoryUsed, stats->rtMemoryUsed);
//		ImGui::Text("Num Programs %u, Num Shaders %u, Uniforms %u", stats->numPrograms, stats->numShaders, stats->numUniforms);
//		ImGui::Text("Num Occlusion Queries %u", stats->numOcclusionQueries);
//
//		ImGui::Columns(1);
//
//		auto& entry = game.GetProfiler()._entries[game.GetProfiler().GetEntryIndex(-1)];
//
//		ImGuiWidgetFlameGraph::PlotFlame(
//			"CPU",
//			[](float* startTimestamp, float* endTimestamp, ImU8* level, const char** caption, const void* data,
//				int idx) -> void {
//					auto entry = reinterpret_cast<const Profiler::Entry*>(data);
//					auto& stage = entry->_stages[idx];
//					if (startTimestamp)
//					{
//						std::chrono::duration<float, std::milli> fltStart = stage._start - entry->_frameStart;
//						*startTimestamp = fltStart.count();
//					}
//					if (endTimestamp)
//					{
//						*endTimestamp = stage._end.time_since_epoch().count() / 1e6f;
//
//						std::chrono::duration<float, std::milli> fltEnd = stage._end - entry->_frameStart;
//						*endTimestamp = fltEnd.count();
//					}
//					if (level)
//					{
//						*level = stage._level;
//					}
//					if (caption)
//					{
//						*caption = Profiler::stageNames[idx].data();
//					}
//			},
//			&entry, static_cast<uint8_t>(Profiler::Stage::_count), 0, "Main Thread", 0, FLT_MAX, ImVec2(width, 0));
//
//		ImGuiWidgetFlameGraph::PlotFlame(
//			"GPU",
//			[](float* startTimestamp, float* endTimestamp, ImU8* level, const char** caption, const void* data,
//				int idx) -> void {
//					auto stats = reinterpret_cast<const bgfx::Stats*>(data);
//					if (startTimestamp)
//					{
//						*startTimestamp =
//							1000.0f * (stats->viewStats[idx].gpuTimeBegin - stats->gpuTimeBegin) / (double)stats->gpuTimerFreq;
//					}
//					if (endTimestamp)
//					{
//						*endTimestamp =
//							1000.0f * (stats->viewStats[idx].gpuTimeEnd - stats->gpuTimeBegin) / (double)stats->gpuTimerFreq;
//					}
//					if (level)
//					{
//						*level = 0;
//					}
//					if (caption)
//					{
//						*caption = stats->viewStats[idx].name;
//					}
//			},
//			stats, stats->numViews, 0, "GPU Frame", 0,
//				1000.0f * (stats->gpuTimeEnd - stats->gpuTimeBegin) / (double)stats->gpuTimerFreq, ImVec2(width, 0));
//
//		ImGui::Columns(2);
//		if (ImGui::CollapsingHeader("Details (CPU)", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			std::chrono::duration<float, std::milli> frameDuration = entry._frameEnd - entry._frameStart;
//			ImGui::Text("Full Frame: %0.3f", frameDuration.count());
//			auto cursorX = ImGui::GetCursorPosX();
//			auto indentSize = ImGui::CalcTextSize("    ").x;
//
//			for (uint8_t i = 0; i < static_cast<uint8_t>(Profiler::Stage::_count); ++i)
//			{
//				std::chrono::duration<float, std::milli> duration = entry._stages[i]._end - entry._stages[i]._start;
//				ImGui::SetCursorPosX(cursorX + indentSize * entry._stages[i]._level);
//				ImGui::Text("    %s: %0.3f", Profiler::stageNames[i].data(), duration.count());
//				if (entry._stages[i]._level == 0)
//					frameDuration -= duration;
//			}
//			ImGui::Text("    Unaccounted: %0.3f", frameDuration.count());
//		}
//		ImGui::NextColumn();
//		if (ImGui::CollapsingHeader("Details (GPU)", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			auto frameDuration = stats->gpuTimeEnd - stats->gpuTimeBegin;
//			ImGui::Text("Full Frame: %0.3f", 1000.0f * frameDuration / (double)stats->gpuTimerFreq);
//
//			for (uint8_t i = 0; i < stats->numViews; ++i)
//			{
//				auto const& viewStat = stats->viewStats[i];
//				int64_t gpuTimeElapsed = viewStat.gpuTimeEnd - viewStat.gpuTimeBegin;
//
//				ImGui::Text("    %s: %0.3f", viewStat.name, 1000.0f * gpuTimeElapsed / (double)stats->gpuTimerFreq);
//				frameDuration -= gpuTimeElapsed;
//			}
//			ImGui::Text("    Unaccounted: %0.3f", 1000.0f * frameDuration / (double)stats->gpuTimerFreq);
//		}
//		ImGui::Columns(1);
//	}
//	ImGui::End();
//}
