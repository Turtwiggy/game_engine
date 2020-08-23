#pragma once

#include "engine/core/game_window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#include "imgui.h"

#include <array>
#include <memory>

namespace fightingengine {

    class ImGui_Manager
    {
    public:
        ~ImGui_Manager();

        void initialize(GameWindow* window);
        void begin(const GameWindow& window);
        void end(const GameWindow& window);

        bool ProcessEventSdl2(const SDL_Event& event);

    private:

        //static const char* StaticGetClipboardText(void* ud) { return reinterpret_cast<ImGui_Manager*>(ud)->GetClipboardText(); }
        //static void StaticSetClipboardText(void* ud, const char* text) { reinterpret_cast<ImGui_Manager*>(ud)->SetClipboardText(text); }
        //const char* GetClipboardText();
        //void SetClipboardText(const char* text);

    private:
        //std::array<bool, 3> _mousePressed;
        //std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> _mouseCursors;
        //char* _clipboardTextData;
        //int64_t _last;
        //int32_t _lastScroll;
    };
}

