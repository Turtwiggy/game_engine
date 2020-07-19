#include "graphics/render_command.h"

namespace fightinggame {
    Scope<renderer_api> RenderCommand::s_RendererAPI = renderer_api::create();
}
