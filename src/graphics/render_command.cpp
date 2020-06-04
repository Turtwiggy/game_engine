#include "graphics/render_command.h"

namespace fightinggame {
    Scope<renderer_api> render_command::s_RendererAPI = renderer_api::Create();
}