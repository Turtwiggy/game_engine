// main.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>

#include <imgui.h>
#include <entt/entt.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 

#include "audio_player.h"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

constexpr int windowWidth{ 800 }, windowHeight{ 600 };

int main()
{
    entt::registry registry;
    std::uint64_t dt = 16;
    glm::vec3(0.0f, 0.0f, 0.0f);

    RenderWindow window{ {windowWidth, windowHeight}, "Game!" };
    window.setFramerateLimit(60);

    audio_player audio;

    //Game loop
    while (true)
    {
        window.clear(Color::Black);

        if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

        if (Keyboard::isKeyPressed(Keyboard::Key::Enter))
            audio.play_oneoff_sound();

        window.display();
    }

    return 0;
}