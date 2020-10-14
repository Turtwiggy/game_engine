#pragma once

//c system headers
#include <cstdint>

//c++ standard library header
#include <array>
#include <chrono>
#include <map>
#include <string_view>

namespace fightingengine
{
class Profiler
{
public:
    enum class Stage : uint8_t
    {
        SdlInput,
        GameTick,
        Render,
        GuiLoop,
        FrameEnd,
        UpdateLoop,

        _count,
    };

public:
    //Each "DeltaTime" has to belong to an "Entry"
    //It represents the time elapsed between calling 
    //profiler.begin(STAGE) and profiler.end(STAGE)
    struct DeltaTime
    {
        std::chrono::system_clock::time_point _start;
        std::chrono::system_clock::time_point _end;
        bool scope_time_finalized = false;
    };

    static constexpr std::array<std::string_view, static_cast<uint8_t>(Stage::_count)> stageNames = {
        "SDL Input",
        "Game Tick",
        "Render",
        "GUI Loop",
        "Frame End",
        "Update Loop"
    };

    //Each "Entry" contains a "DeltaTime" value 
    //for each "Stage" for the profiler, so that 
    //every frame for the application has one "Entry".
    struct Entry
    {
        std::chrono::system_clock::time_point frame_start;
        std::chrono::system_clock::time_point frame_end;
        std::array<DeltaTime, static_cast<uint8_t>(Stage::_count)> stages;
    };

    void new_frame();
    void begin(const Stage& stage);
    void end(const Stage& stage);

    //returns milliseconds the profiler stage took this frame
    [[nodiscard]] float get_time(const Stage& request);
    //returns average milliseconds the the last "frames_data_live" frames took
    [[nodiscard]] float get_average_time(const Stage& request);

private:
    uint8_t get_entry_index(int8_t offset) const;

    //a buffer for the profiler entries 
    //i.e. how many frames to keep the data
    //Currently storing 60 frames so the "average" function 
    //has some data to work with
    static constexpr uint8_t frames_data_live = 60;
    std::array<Entry, frames_data_live> entries;
    
    uint8_t current_entry = frames_data_live - 1;
};

} //namespace fightingengine