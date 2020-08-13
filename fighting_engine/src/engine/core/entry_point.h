#pragma once

#include "engine/core/application.h"
#undef main //thanks sdl2

extern fightingengine::Application* fightingengine::CreateApplication();

int main(int argc, char** argv)
{
    auto app = fightingengine::CreateApplication();

    app->run();

    delete app;
}
