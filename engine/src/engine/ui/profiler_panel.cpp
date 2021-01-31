
// header
#include "engine/ui/profiler_panel.hpp"

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
  bool animate = true;

  float values[90] = {};
  int values_offset = 0;
  double refresh_time = 0.0;

public:
  float scale_min = 0.0f;
  float scale_max = 50.0f;

public:
  void draw(float next_value)
  {
    //
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
      sprintf_s(overlay, "avg %f", average);
      ImGui::PlotLines(
        "Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, scale_min, scale_max, ImVec2(0, 30.0f));
    }

    ImGui::Columns(1);
  }
};

void
draw(const Profiler& profiler, const float delta_time_s)
{
  ImGui::Begin("Profiler");

  static AnimatedProfilerEntry framerate;
  framerate.draw(ImGui::GetIO().Framerate);
  ImGui::Text("%s %f ms", "Framerate: ", ImGui::GetIO().Framerate);
  ImGui::Separator();

  float time = profiler.get_average_time(Profiler::Stage::SdlInput);
  static AnimatedProfilerEntry input;
  input.scale_max = 300.0f; // an upper framerate
  input.draw(time);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::SdlInput].data(), (time));
  ImGui::Separator();

  time = profiler.get_average_time(Profiler::Stage::GameTick);
  static AnimatedProfilerEntry game_tick;
  game_tick.draw(time);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GameTick].data(), (time));
  ImGui::Separator();

  time = profiler.get_average_time(Profiler::Stage::Render);
  static AnimatedProfilerEntry render;
  render.draw(time);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::Render].data(), (time));
  ImGui::Separator();

  time = profiler.get_average_time(Profiler::Stage::GuiLoop);
  static AnimatedProfilerEntry gui_loop;
  gui_loop.draw(time);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::GuiLoop].data(), (time));
  ImGui::Separator();

  time = profiler.get_average_time(Profiler::Stage::FrameEnd);
  static AnimatedProfilerEntry frame_end;
  frame_end.draw(time);
  ImGui::Text("%s %f ms", profiler.stageNames[(uint8_t)Profiler::Stage::FrameEnd].data(), (time));
  ImGui::Separator();

  time = profiler.get_average_time(Profiler::Stage::UpdateLoop);
  static AnimatedProfilerEntry update_loop;
  update_loop.draw(time);
  ImGui::Text("~~ %s %f ms ~~", profiler.stageNames[(uint8_t)Profiler::Stage::UpdateLoop].data(), (time));
  ImGui::Separator();

  ImGui::End();
}

} // namespace profiler_panel

} // namespace fightingengine