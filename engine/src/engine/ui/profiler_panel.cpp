
// header
#include "engine/ui/profiler_panel.hpp"

// other library headers
#include <imgui.h>

using namespace fightingengine;

namespace fightingengine {

namespace profiler_panel {

void
draw(const Profiler& profiler, const float delta_time_s)
{
  ImGui::Begin("Profiler");

  // Fill an array of contiguous float values to plot
  // Tip: If your float aren't contiguous but part of a structure, you can pass
  // a pointer to your first float and the sizeof() of your structure in the
  // "stride" parameter.
  static bool animate = true;
  ImGui::Checkbox("Animate", &animate);

  static float values[90] = {};
  static int values_offset = 0;
  static double refresh_time = 0.0;
  if (!animate || refresh_time == 0.0)
    refresh_time = ImGui::GetTime();
  while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
  {
    values[values_offset] = ImGui::GetIO().Framerate;
    values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
    refresh_time += 1.0f / 60.0f;
  }

  {
    float average = 0.0f;
    for (int n = 0; n < IM_ARRAYSIZE(values); n++)
      average += values[n];
    average /= (float)IM_ARRAYSIZE(values);
    char overlay[32];
    sprintf_s(overlay, "avg %f", average);
    ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, 144.0f, ImVec2(0, 40.0f));
  }

  ImGui::Columns(1);

  float time = profiler.get_average_time(Profiler::Stage::SdlInput);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::SdlInput].data(), (time));

  time = profiler.get_average_time(Profiler::Stage::GameTick);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GameTick].data(), (time));

  time = profiler.get_average_time(Profiler::Stage::Render);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::Render].data(), (time));

  time = profiler.get_average_time(Profiler::Stage::GuiLoop);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GuiLoop].data(), (time));

  time = profiler.get_average_time(Profiler::Stage::FrameEnd);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::FrameEnd].data(), (time));

  time = profiler.get_average_time(Profiler::Stage::UpdateLoop);
  ImGui::Text("~~ %s %f ms ~~", profiler.stageNames[(uint8_t)Profiler::Stage::UpdateLoop].data(), (time));

  ImGui::End();
}

} // namespace profiler_panel

} // namespace fightingengine