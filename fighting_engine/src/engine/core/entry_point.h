#pragma once

extern fightingengine::Application* Hazel::CreateApplication();

int main(int argc, char** argv)
{
    auto app = fightingengine::CreateApplication();

    app->Run();

    delete app;
}
