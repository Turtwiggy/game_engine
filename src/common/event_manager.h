#include <common/queue.h>
#include <SDL2/SDL.h>
#include <functional>
#include <stdexcept>
#include <unordered_map>

#pragma once

namespace fightinggame
{

	class event_manager;

	class event_manager
	{
	public:
		template <typename Event, typename... Args>
		void Create(Event event)
		{
			Insist<Event>()->Produce(event);
		}

		template <typename Event>
		void AddHandler(EventHandler<Event> callback)
		{
			Insist<Event>()->AddHandler(callback);
		}

	private:
		class _EventFamily
		{
		private:
			inline static std::atomic<int> identifier = 0;

		public:
			template <typename... T>
			inline static const int Type = identifier++;
		};
		mutable std::unordered_map<int, std::unique_ptr<IEventQueue>> queues{};
		template <typename Event>
		event_queue<Event>* Insist()
		{
			const auto eventType = event_manager::_EventFamily::Type<Event>;
			std::unique_ptr<IEventQueue>& p = queues[eventType];

			if (!p)
				p.reset(new event_queue<Event>());

			return static_cast<event_queue<Event>*>(p.get());
		}
	};
}