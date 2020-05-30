#include "renderer.h"

#include "window/game_window.h"

#include <bx/os.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <spdlog/spdlog.h>

#include "logo.hpp"

using namespace fightinggame;
using namespace fightinggame::graphics;

namespace fightinggame
{
    renderer::renderer(const game_window* window, bgfx::RendererType::Enum rendererType, bool vsync)
    {
        bgfx::Init init{};
        init.type = rendererType;

        // Get render area size
        int drawable_width;
        int drawable_height;
        if (rendererType != bgfx::RendererType::Noop)
        {
            window->GetSize(drawable_width, drawable_height);
            spdlog::info("rendering at {}, {}", drawable_width, drawable_height);

            init.resolution.width = static_cast<uint32_t>(drawable_width);
            init.resolution.height = static_cast<uint32_t>(drawable_height);

            // Get Native Handles from SDL window
            window->GetNativeHandles(init.platformData.nwh, init.platformData.ndt);
        }

        init.resolution.reset = BGFX_RESET_NONE;
        if (vsync)
        {
            init.resolution.reset |= BGFX_RESET_VSYNC;
        }
        //init.callback = dynamic_cast<bgfx::CallbackI*>(_bgfxCallback.get());

        if (!bgfx::init(init))
        {
            throw std::runtime_error("Failed to initialize bgfx.");
        }

        //LoadShaders();

        //// allocate vertex buffers for our debug draw
        //_debugCross = DebugLines::CreateCross();

        // give debug names to views
        for (bgfx::ViewId i = 0; i < static_cast<bgfx::ViewId>(graphics::render_pass::_count); ++i)
        {
            bgfx::setViewName(i, RenderPassNames[i].data());
        }
    }

    renderer::~renderer()
    {
        //_shaderManager.reset();
        //_debugCross.reset();
        bgfx::shutdown();
    }

    void renderer::configure_view(graphics::render_pass view_id, uint16_t width, uint16_t height) const
    {
        static const uint32_t clearColor = 0x274659ff;

        bgfx::setViewClear(static_cast<bgfx::ViewId>(view_id), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColor, 1.0f, 0);
        bgfx::setViewRect(static_cast<bgfx::ViewId>(view_id), 0, 0, width, height);
    }

    void renderer::draw_scene(const draw_scene_desc& desc) const
    {
        //// Reflection Pass
        //{
        //	auto section = drawDesc.profiler.BeginScoped(Profiler::Stage::ReflectionPass);
        //	if (drawDesc.drawWater)
        //	{
        //		DrawSceneDesc drawPassDesc = drawDesc;

        //		auto& frameBuffer = drawDesc.water.GetFrameBuffer();
        //		auto reflectionCamera = drawDesc.camera->Reflect(drawDesc.water.GetReflectionPlane());

        //		drawPassDesc.viewId = graphics::RenderPass::Reflection;
        //		drawPassDesc.camera = reflectionCamera.get();
        //		drawPassDesc.frameBuffer = &frameBuffer;
        //		drawPassDesc.drawWater = false;
        //		drawPassDesc.drawDebugCross = false;
        //		drawPassDesc.drawBoundingBoxes = false;
        //		drawPassDesc.cullBack = true;
        //		DrawPass(meshPack, drawPassDesc);
        //	}
        //}

        //// Main Draw Pass
        //{
        //	auto section = drawDesc.profiler.BeginScoped(Profiler::Stage::MainPass);
        //	DrawPass(meshPack, drawDesc);
        //}

        draw_pass(desc);
    }

    void renderer::draw_pass(const draw_scene_desc& desc) const
    {
        bgfx::setViewRect(0, 0, 0, uint16_t(desc.width), uint16_t(desc.height));

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::touch(static_cast<bgfx::ViewId>(desc.view_id));

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextImage(
            bx::max<uint16_t>(uint16_t(desc.width / 2 / 8), 20) - 20
            , bx::max<uint16_t>(uint16_t(desc.height / 2 / 16), 6) - 6
            , 40
            , 12
            , s_logo
            , 160
        );
        bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

        bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
        bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

        const bgfx::Stats* stats = bgfx::getStats();
        bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
            , stats->width
            , stats->height
            , stats->textWidth
            , stats->textHeight
        );

        //bgfx::submit(static_cast<bgfx::ViewId>(desc.view_id));

        // Enable stats or debug text.
        auto debugMode = BGFX_DEBUG_NONE;
        //if (desc.bgfxDebug)
        //{
        //    debugMode |= BGFX_DEBUG_STATS;
        //}
        //if (desc.wireframe)
        //{
        //    debugMode |= BGFX_DEBUG_WIREFRAME;
        //}
        //if (desc.profile)
        //{
        //    debugMode |= BGFX_DEBUG_PROFILER;
        //}
        bgfx::setDebug(BGFX_DEBUG_TEXT);

    }

    void renderer::frame()
    {
        // Advance to next frame. Process submitted rendering primitives.
        bgfx::frame();
    }
}