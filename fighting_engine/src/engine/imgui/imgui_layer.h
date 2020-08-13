#pragma once

#include "engine/core/layer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

namespace fightingengine {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer() = default;

        virtual void on_attach() override;
        virtual void on_detach() override;
        virtual void on_event(Event& e) override;

        void begin();
        void end();

        bool ProcessEventSdl2(const SDL_Event& event);
        void BlockEvents(bool block) { m_BlockEvents = block; }
    private:

        static const char* StaticGetClipboardText(void* ud) { return reinterpret_cast<ImGuiLayer*>(ud)->GetClipboardText(); }
        static void StaticSetClipboardText(void* ud, const char* text) { reinterpret_cast<ImGuiLayer*>(ud)->SetClipboardText(text); }
        const char* GetClipboardText();
        void SetClipboardText(const char* text);


    private:
        bool m_BlockEvents = true;
        float m_Time = 0.0f;

        std::array<bool, 3> _mousePressed;
        std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> _mouseCursors;
        char* _clipboardTextData;
        int64_t _last;
        int32_t _lastScroll;
    };

}

