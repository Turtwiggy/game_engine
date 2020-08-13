#pragma once

#include "engine/events/event.h"

namespace fightingengine {

    class Layer
    {
    public:
        Layer(const std::string& name = "DefaultLayer");
        virtual ~Layer() = default;

        virtual void on_attach() {}
        virtual void on_detach() {}
        virtual void on_update(float delta_time) {}
        virtual void on_imgui_render() {}
        virtual void on_event(Event& event) {}

        const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };

}
