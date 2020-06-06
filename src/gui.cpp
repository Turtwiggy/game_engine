#include "gui.hpp"

#include "game.h"
#include "tools/profiler.hpp"
#include "window/game_window.h"

#include <imgui.h>
#include <imgui_widget_flamegraph.h>
#include <examples/imgui_impl_sdl.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif
#include <GL/glew.h>

using namespace fightinggame;

#define IMGUI_FLAGS_NONE UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

Gui::Gui()
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

bool Gui::Loop(game& game, ImGuiContext* imgui, profiler& profiler)
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
        ShowProfilerWindow(game, profiler);

    ImGui::Render();

    return false;
}

void Gui::ShowProfilerWindow(game& game, profiler& profiler)
{
    ImGui::Begin("Profiler");

    ImGui::Columns(1);

    auto width = ImGui::GetColumnWidth() - ImGui::CalcTextSize("Frame").x;

    auto& entry = profiler._entries[profiler.GetEntryIndex(-1)];

    ImGuiWidgetFlameGraph::PlotFlame(
        "CPU",
        [](float* startTimestamp, float* endTimestamp, ImU8* level, const char** caption, const void* data,
            int idx) -> void {
                auto entry = reinterpret_cast<const profiler::Entry*>(data);
                auto& stage = entry->_stages[idx];
                if (startTimestamp)
                {
                    std::chrono::duration<float, std::milli> fltStart = stage._start - entry->_frameStart;
                    *startTimestamp = fltStart.count();
                }
                if (endTimestamp)
                {
                    *endTimestamp = stage._end.time_since_epoch().count() / 1e6f;

                    std::chrono::duration<float, std::milli> fltEnd = stage._end - entry->_frameStart;
                    *endTimestamp = fltEnd.count();
                }
                if (level)
                {
                    *level = stage._level;
                }
                if (caption)
                {
                    *caption = profiler::stageNames[idx].data();
                }
        },
        &entry, static_cast<uint8_t>(profiler::Stage::_count), 0, "Main Thread", 0, FLT_MAX, ImVec2(width, 0));

    ImGui::End();
}

