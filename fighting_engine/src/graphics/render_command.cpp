#include "graphics/render_command.h"

namespace fightingengine {
    std::unique_ptr<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::create();
}
