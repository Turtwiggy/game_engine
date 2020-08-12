#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace fightingengine {

    template <typename Event>
    using EventHandler = std::function<void(const Event&)>;

    class IEventQueue
    {
    public:
        IEventQueue() {};
        virtual ~IEventQueue() {};
    };

    template <class T>
    class EventQueue : public IEventQueue
    {
    public:
        using Ptr = std::shared_ptr<EventQueue>;

        EventQueue() {}
        void AddHandler(EventHandler<T> handler) { _handlers.push_back(handler); }

        void Produce(T event)
        {
            _instance.push_back(event);

            for (auto handler : _handlers)
            {
                handler(_instance.front());
            }

            _instance.pop_front();
        }

    private:
        ~EventQueue() {}

        using Queue = std::list<T>;
        Queue _instance;
        std::vector<EventHandler<T>> _handlers;
    };

}
