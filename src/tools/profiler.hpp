#pragma once

#include "imgui.h"
#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <string_view>

namespace fightinggame
{
class profiler
{
public:
    enum class Stage : uint8_t
    {
        SdlInput,
        NewFrame,
        //UpdateUniforms,
        //UpdateEntities,
        GuiLoop,
        SceneDraw,
        //MainPass,
        RenderFrame,

        _count,
    };

private:
    struct ScopedSection
    {
        inline explicit ScopedSection(profiler* profiler, Stage stage)
            : _profiler(profiler)
            , _stage(stage)
        {
            _profiler->Begin(_stage);
        }
        inline ~ScopedSection() { _profiler->End(_stage); }

        profiler* const _profiler;
        const Stage _stage;
    };

public:
    struct Scope
    {
        uint8_t _level;
        std::chrono::system_clock::time_point _start;
        std::chrono::system_clock::time_point _end;
        bool _finalized = false;
    };

    static constexpr std::array<std::string_view, static_cast<uint8_t>(Stage::_count)> stageNames = {
        "SDL Input",
        "New Frame",
        //"Update Uniforms",
        //"Update Entities",
        "GUI Loop",
        "Scene Draw",
        //"Main Pass",
        "Renderer Frame",
    };

    struct Entry
    {
        std::chrono::system_clock::time_point _frameStart;
        std::chrono::system_clock::time_point _frameEnd;
        std::array<Scope, static_cast<uint8_t>(Stage::_count)> _stages;
    };

    void Frame();
    void Begin(Stage stage);
    void End(Stage stage);
    inline ScopedSection BeginScoped(Stage stage) { return ScopedSection(this, stage); }

    [[nodiscard]] uint8_t GetEntryIndex(int8_t offset) const { return (_currentEntry + _bufferSize + offset) % _bufferSize; }

    static constexpr uint8_t _bufferSize = 100;
    std::array<Entry, _bufferSize> _entries;

private:
    uint8_t _currentEntry = _bufferSize - 1;
    uint8_t _currentLevel = 0;
};
}

