
// header
#include "engine/ui/profiler_panel.hpp"

// standard lib headers
// clang-format off
#include <string>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"
#include "psapi.h"
#endif
// clang-forrmat on

// other library headers
#include <imgui.h>

using namespace fightingengine;

namespace fightingengine {

namespace profiler_panel {

class AnimatedProfilerEntry
{
  // Fill an array of contiguous float values to plot
  // Tip: If your float aren't contiguous but part of a structure, you can pass
  // a pointer to your first float and the sizeof() of your structure in the
  // "stride" parameter.
  bool show_animate_tickbox = false;
  bool animate = true;

  float values[90] = {};
  int values_offset = 0;
  double refresh_time = 0.0;

public:
  std::string name = "default";
  float scale_min = 0.0f;
  float scale_max = 50.0f;

public:
  void draw(float next_value)
  {
    //
    if(show_animate_tickbox)
      ImGui::Checkbox("Animate", &animate);
    if (!animate || refresh_time == 0.0)
      refresh_time = ImGui::GetTime();

    while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
    {
      values[values_offset] = next_value;
      values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
      refresh_time += 1.0f / 60.0f;
    }

    {
      float average = 0.0f;
      for (int n = 0; n < IM_ARRAYSIZE(values); n++)
        average += values[n];
      average /= (float)IM_ARRAYSIZE(values);
      char overlay[32];
      sprintf_s(overlay, "%s %f", name.c_str(), average);
      ImGui::PlotLines(
        "Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, scale_min, scale_max, ImVec2(0, 30.0f));
    }

    ImGui::Columns(1);
  }
};

void
draw_timers(const Profiler& profiler, const float delta_time_s)
{

  //
  // Game Time info
  //


  float time = profiler.get_average_time(Profiler::Stage::SdlInput);
  static AnimatedProfilerEntry sdl_input;
  sdl_input.draw(time);
  sdl_input.name = std::string("sdl input");
  // ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GuiLoop].data(), (time));

  time = profiler.get_average_time(Profiler::Stage::Physics);
  static AnimatedProfilerEntry physics;
  physics.draw(time);
  physics.name = std::string("physics");

  time = profiler.get_average_time(Profiler::Stage::GameTick);
  static AnimatedProfilerEntry game_tick;
  game_tick.draw(time);
  game_tick.name = std::string("game tick");

  time = profiler.get_average_time(Profiler::Stage::Render);
  static AnimatedProfilerEntry render;
  render.draw(time);
  render.name = std::string("render");

  time = profiler.get_average_time(Profiler::Stage::GuiLoop);
  static AnimatedProfilerEntry gui;
  gui.draw(time);
  gui.name = std::string("gui");

  time = profiler.get_average_time(Profiler::Stage::FrameEnd);
  static AnimatedProfilerEntry end;
  end.draw(time);
  end.name = std::string("frameend");

  time = profiler.get_average_time(Profiler::Stage::UpdateLoop);
  ImGui::Text("~~ %s %f ms ~~", profiler.stageNames[(uint8_t)Profiler::Stage::UpdateLoop].data(), (time));
  ImGui::Separator();

  //
  // Memory Usage Info
  //

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

  // https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
  DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
  DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;

  PROCESS_MEMORY_COUNTERS_EX pmc;
  GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
  SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

  DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
  DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
  SIZE_T physMemUsedByMe = pmc.WorkingSetSize;

  ImGui::Text("Your Virtual Memory: %s", std::to_string(static_cast<uint64_t>(totalVirtualMem)).c_str());
  ImGui::Text("Used Virtual Memory: %s", std::to_string(static_cast<uint64_t>(virtualMemUsed)).c_str());
  ImGui::Text("FG's Used Virtual Memory: %s", std::to_string(static_cast<uint64_t>(virtualMemUsedByMe)).c_str());

  ImGui::Text("Your Physical Memory: %s", std::to_string(static_cast<uint64_t>(totalPhysMem)).c_str());
  ImGui::Text("Used Physical Memory: %s", std::to_string(static_cast<uint64_t>(physMemUsed)).c_str());
  ImGui::Text("FG's Used Physical Memory: %s", std::to_string(static_cast<uint64_t>(physMemUsedByMe)).c_str());

#endif
}

} // namespace profiler_panel

} // namespace fightingengine