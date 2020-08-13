#pragma once

#include "engine/core/layer.h"

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

        void BlockEvents(bool block) { m_BlockEvents = block; }
    private:
        bool m_BlockEvents = true;
        float m_Time = 0.0f;
    };

}

