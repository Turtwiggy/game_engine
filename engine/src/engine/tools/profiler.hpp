// #pragma once

// #include "imgui.h"
// #include "common/circular_buffer.h"
// #include <array>
// #include <chrono>
// #include <cstdint>
// #include <map>
// #include <string_view>

// namespace fightinggame
// {
//     class profiler
//     {
//     public:
//         enum class Stage : uint8_t
//         {
//             SdlInput,
//             NewFrame,
//             //UpdateUniforms,
//             //UpdateEntities,
//             GuiLoop,
//             SceneDraw,
//             RenderFrame,
//             UpdateLoop,

//             _count,
//         };

//     public:
//         struct Scope
//         {
//             uint8_t _level;
//             std::chrono::system_clock::time_point _start;
//             std::chrono::system_clock::time_point _end;
//             bool _finalized = false;
//         };

//         static constexpr std::array<std::string_view, static_cast<uint8_t>(Stage::_count)> stageNames = {
//             "SDL Input",
//             "New Frame",
//             //"Update Uniforms",
//             //"Update Entities",
//             "GUI Loop",
//             "Scene Draw",
//             "Renderer Frame",
//             "Update Loop",
//         };

//         struct Entry
//         {
//             std::chrono::system_clock::time_point _frameStart;
//             std::chrono::system_clock::time_point _frameEnd;
//             std::array<Scope, static_cast<uint8_t>(Stage::_count)> _stages;
//         };

//         void Frame();
//         void Begin(Stage stage);
//         void End(Stage stage);

//         [[nodiscard]] uint8_t GetEntryIndex(int8_t offset) const { return (_currentEntry + _bufferSize + offset) % _bufferSize; }

//         //returns milliseconds the profiler stage took
//         float GetTime(Stage request)
//         {
//             auto& entry = _entries[GetEntryIndex(-1)];
//             auto& stage = entry._stages[(int)request];

//             std::chrono::duration<float, std::milli> fltStart = stage._start - entry._frameStart;
//             float startTimestamp = fltStart.count();

//             std::chrono::duration<float, std::milli> fltEnd = stage._end - entry._frameStart;
//             float endTimestamp = fltEnd.count();

//             auto& buf = buffer[(int)request];
//             buf.add_next(endTimestamp - startTimestamp);
//             return buf.average();
//         }

//         //a buffer for the profiler entries
//         static constexpr uint8_t _bufferSize = 20;
//         std::array<Entry, _bufferSize> _entries;

//         //a buffer for the results of the profiler entries
//         std::array<circular_buffer, static_cast<uint8_t>(Stage::_count)> buffer;

//     private:
//         uint8_t _currentEntry = _bufferSize - 1;
//         uint8_t _currentLevel = 0;
//     };
// }