#include "graphics/render_command.h"

namespace fightinggame {
    std::unique_ptr<renderer_api> RenderCommand::s_RendererAPI = renderer_api::create();
}
