#pragma once

// ImGui Bezier widget. @r-lyeh, public domain
// v1.03: improve grabbing, confine grabbers to area option, adaptive size, presets, preview.
// v1.02: add BezierValue(); comments; usage
// v1.01: out-of-bounds coord snapping; custom border width; spacing; cosmetics
// v1.00: initial version
//
// [ref] http://robnapier.net/faster-bezier
// [ref] http://easings.net/es#easeInSine
//

////Bezier Usage:
//static float sample_x = 0.5f;
//static float start_point[2] = { 0.f, 0.f };
//static float middle_points[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
//static float end_point[2] = { 1.f, 0.f };
//
////In Update Function:
//ImGui::SliderFloat("Bezier Sample Value", &sample_x, 0.0f, 1.0f, "ratio = %.3f");
//ImGui::SliderFloat2("Bezier Start Point", start_point, 0.f, 1.f, "ratio = %.3f");
//ImGui::SliderFloat2("Bezier End Point", end_point, 0.f, 1.f, "ratio = %.3f");
//ImGui::Bezier("Bezier Control", sample_x, middle_points, start_point, end_point);

#define IMGUI_DEFINE_MATH_OPERATORS

//c system headers
#include <time.h>

//c++ standard library headers
#include <vector>

//other library headers
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/glm.hpp>


namespace ImGui
{
glm::vec2 EvaluateQuadratic(glm::vec2 a, glm::vec2 b, glm::vec2 c, float t)
{
    glm::vec2 p0 = mix(a, b, t);
    glm::vec2 p1 = mix(b, c, t);
    return mix(p0, p1, t);
}

glm::vec2 EvaluateCubic(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float t)
{
    glm::vec2 p0 = EvaluateQuadratic(a, b, c, t);
    glm::vec2 p1 = EvaluateQuadratic(b, c, d, t);
    glm::vec2 val = mix(p0, p1, t);
    return val;
}

template<int steps>
void bezier_table(std::vector<glm::vec2> P, glm::vec2 results[steps + 1])
{
    for (unsigned step = 0; step <= steps; ++step)
    {
        float t = (float)step / (float)steps;
        results[step] = EvaluateCubic(P[0], P[1], P[2], P[3], t);
    }
}

float BezierValue(std::vector<glm::vec2> P, float t)
{
    glm::vec2 val = EvaluateCubic(P[0], P[1], P[2], P[3], t);
    return val.y;
}

int Bezier(const char* label, float dt, float middle_points[4], float start_point[2], float end_point[2]) {
    // visuals
    enum { SMOOTHNESS = 128 }; // curve smoothness: the higher number of segments, the smoother curve
    enum { CURVE_WIDTH = 4 }; // main curved line width
    enum { LINE_WIDTH = 1 }; // handlers: small lines width
    enum { GRAB_RADIUS = 8 }; // handlers: circle radius
    enum { GRAB_BORDER = 2 }; // handlers: circle border width
    enum { AREA_CONSTRAINED = true }; // should grabbers be constrained to grid area?
    enum { AREA_WIDTH = 128 }; // area width in pixels. 0 for adaptive size (will use max avail width)

    // bezier widget

    const ImGuiStyle& Style = GetStyle();
    const ImGuiIO& IO = GetIO();
    ImDrawList* DrawList = GetWindowDrawList();
    ImGuiWindow* Window = GetCurrentWindow();
    if (Window->SkipItems)
        return false;

    // header and spacing
    int changed = SliderFloat4(label, middle_points, 0, 1, "%.3f", 1.0f);
    int hovered = IsItemActive() || IsItemHovered(); // IsItemDragged() ?
    Dummy(Imglm::vec2(0, 3));

    // prepare canvas
    const float avail = GetContentRegionAvailWidth();
    const float dim = AREA_WIDTH > 0 ? AREA_WIDTH : avail;
    Imglm::vec2 Canvas(dim, dim);

    ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
    ItemSize(bb);
    if (!ItemAdd(bb, NULL))
        return changed;

    const ImGuiID id = Window->GetID(label);
    hovered |= 0 != ItemHoverable(ImRect(bb.Min, bb.Min + Imglm::vec2(avail, dim)), id);

    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

    // background grid
    for (int i = 0; i <= Canvas.x; i += (Canvas.x / 4)) {
        DrawList->AddLine(
            Imglm::vec2(bb.Min.x + i, bb.Min.y),
            Imglm::vec2(bb.Min.x + i, bb.Max.y),
            GetColorU32(ImGuiCol_TextDisabled));
    }
    for (int i = 0; i <= Canvas.y; i += (Canvas.y / 4)) {
        DrawList->AddLine(
            Imglm::vec2(bb.Min.x, bb.Min.y + i),
            Imglm::vec2(bb.Max.x, bb.Min.y + i),
            GetColorU32(ImGuiCol_TextDisabled));
    }

    // eval curve
    std::vector<glm::vec2> Q = std::vector<glm::vec2>
    {
        { start_point[0], start_point[1] },
        { middle_points[0], middle_points[1] },
        { middle_points[2], middle_points[3] },
        { end_point[0], end_point[1] }
    };
    glm::vec2 results[SMOOTHNESS + 1];
    bezier_table<SMOOTHNESS>(Q, results);

    // control points: 2 lines and 2 circles
    {
        // handle grabbers
        Imglm::vec2 mouse = GetIO().MousePos, pos[2];
        float distance[2];

        for (int i = 0; i < 2; ++i) {
            pos[i] = Imglm::vec2(middle_points[i * 2 + 0], 1 - middle_points[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
            distance[i] = (pos[i].x - mouse.x) * (pos[i].x - mouse.x) + (pos[i].y - mouse.y) * (pos[i].y - mouse.y);
        }

        int selected = distance[0] < distance[1] ? 0 : 1;
        if (distance[selected] < (4 * GRAB_RADIUS * 4 * GRAB_RADIUS))
        {
            SetTooltip("(%4.3f, %4.3f)", middle_points[selected * 2 + 0], middle_points[selected * 2 + 1]);

            if (/*hovered &&*/ (IsMouseClicked(0) || IsMouseDragging(0))) {
                float& px = (middle_points[selected * 2 + 0] += GetIO().MouseDelta.x / Canvas.x);
                float& py = (middle_points[selected * 2 + 1] -= GetIO().MouseDelta.y / Canvas.y);

                if (AREA_CONSTRAINED) {
                    px = (px < 0 ? 0 : (px > 1 ? 1 : px));
                    py = (py < 0 ? 0 : (py > 1 ? 1 : py));
                }

                changed = true;
            }
        }
    }

    // if (hovered || changed) DrawList->PushClipRectFullScreen();

    // draw curve
    {
        ImColor color(GetStyle().Colors[ImGuiCol_PlotLines]);
        for (int i = 0; i < SMOOTHNESS; ++i) {
            Imglm::vec2 p = { results[i + 0].x, 1 - results[i + 0].y };
            Imglm::vec2 q = { results[i + 1].x, 1 - results[i + 1].y };
            Imglm::vec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
            Imglm::vec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
            DrawList->AddLine(r, s, color, CURVE_WIDTH);
        }
    }

    // draw preview (cycles every 1s)
    static clock_t epoch = clock();
    ImVec4 white(GetStyle().Colors[ImGuiCol_Text]);
    for (int i = 0; i < 1; ++i) {
        double now = ((clock() - epoch) / (double)CLOCKS_PER_SEC);
        float delta = ((int)(now * 1000) % 1000) / 1000.f; delta += i / 3.f; if (delta > 1) delta -= 1;
        int idx = (int)(delta * SMOOTHNESS);
        float evalx = results[idx].x; //
        float evaly = results[idx].y; // ImGui::BezierValue( delta, P );
        Imglm::vec2 p0 = Imglm::vec2(evalx, 1 - 0) * (bb.Max - bb.Min) + bb.Min;
        Imglm::vec2 p1 = Imglm::vec2(0, 1 - evaly) * (bb.Max - bb.Min) + bb.Min;
        Imglm::vec2 p2 = Imglm::vec2(evalx, 1 - evaly) * (bb.Max - bb.Min) + bb.Min;
        //DrawList->AddCircleFilled(p0, GRAB_RADIUS / 2, ImColor(white));
        //DrawList->AddCircleFilled(p1, GRAB_RADIUS / 2, ImColor(white));
        DrawList->AddCircleFilled(p2, GRAB_RADIUS / 2, ImColor(white));
        continue;
    }

    //Draw the sampled point
    /*float dy01 = BezierValue(Q, dt);
    Imglm::vec2 py = Imglm::vec2(dt, 1- dy01) * (bb.Max - bb.Min) + bb.Min;
    DrawList->AddCircleFilled(py, GRAB_RADIUS / 2, ImColor(1.f, 0.f, 0.f));*/

    // Draw sampled point 2.0
    int didx = dt * SMOOTHNESS;

    if (didx >= SMOOTHNESS)
        didx = SMOOTHNESS - 1;

    if (didx < 0)
        didx = 0;

    float devalx = results[didx].x;
    float devaly = results[didx].y;

    DrawList->AddCircleFilled(Imglm::vec2(devalx, 1 - devaly) * (bb.Max - bb.Min) + bb.Min, GRAB_RADIUS / 2, ImColor(1.f, 0.f, 0.f));

    // draw lines and grabbers
    float luma = IsItemActive() || IsItemHovered() ? 0.5f : 1.0f;
    ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
    Imglm::vec2 p1 = Imglm::vec2(middle_points[0], 1 - middle_points[1]) * (bb.Max - bb.Min) + bb.Min;
    Imglm::vec2 p2 = Imglm::vec2(middle_points[2], 1 - middle_points[3]) * (bb.Max - bb.Min) + bb.Min;
    DrawList->AddLine(Imglm::vec2(bb.Min.x, bb.Max.y), p1, ImColor(white), LINE_WIDTH);
    DrawList->AddLine(Imglm::vec2(bb.Max.x, bb.Min.y), p2, ImColor(white), LINE_WIDTH);
    DrawList->AddCircleFilled(p1, GRAB_RADIUS, ImColor(white));
    DrawList->AddCircleFilled(p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
    DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(white));
    DrawList->AddCircleFilled(p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));

    // if (hovered || changed) DrawList->PopClipRect();

    return changed;
}

} //namespace ImGui
