#pragma once

namespace fightingengine {

    template<typename T>
    class Singleton {
    public:
        static T& instance();

        Singleton(const Singleton&) = delete;
        Singleton& operator= (const Singleton) = delete;

    protected:
        Singleton() {}
    };

#include <memory>
    template<typename T>
    T& Singleton<T>::instance()
    {
        static T instance{};
        return instance;
    }

}
