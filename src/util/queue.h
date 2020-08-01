#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

template <typename Event>
using EventHandler = std::function<void(const Event&)>;

class IEventQueue
{
public:
	IEventQueue() {};
	virtual ~IEventQueue() {};
};

template <class T>
class event_queue : public IEventQueue
{
public:
	using Ptr = std::shared_ptr<event_queue>;

	event_queue() {}
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
	~event_queue() {}

	using Queue = std::list<T>;
	Queue _instance;
	std::vector<EventHandler<T>> _handlers;
};
