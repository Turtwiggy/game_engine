#include "profiler.hpp"

#include <cassert>

namespace fightinggame {

    void profiler::Begin(Stage stage)
    {
        assert(_currentLevel < 255);
        auto& entry = _entries[_currentEntry]._stages[static_cast<uint8_t>(stage)];
        entry._level = _currentLevel;
        _currentLevel++;
        entry._start = std::chrono::system_clock::now();
        entry._finalized = false;
    }

    void profiler::End(Stage stage)
    {
        assert(_currentLevel > 0);
        auto& entry = _entries[_currentEntry]._stages[static_cast<uint8_t>(stage)];
        assert(!entry._finalized);
        _currentLevel--;
        assert(entry._level == _currentLevel);
        entry._end = std::chrono::system_clock::now();
        entry._finalized = true;
    }

    void profiler::Frame()
    {
        auto& prevEntry = _entries[_currentEntry];
        _currentEntry = (_currentEntry + 1) % _bufferSize;
        prevEntry._frameEnd = _entries[_currentEntry]._frameStart = std::chrono::system_clock::now();
    }
}