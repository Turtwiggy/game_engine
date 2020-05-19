// main.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <entt/entt.hpp>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Audio.hpp>
#include <SFML/OpenGL.hpp> //This header includes OpenGL functions, and nothing else.
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "audio_player.hpp"

int main()
{
    entt::registry registry;
    std::uint64_t dt = 16;
    glm::vec3(0.0f, 0.0f, 0.0f);

    sf::RenderWindow window(sf::VideoMode(640, 480), "Fighting Game");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(false);
    window.setActive(true);
    ImGui::SFML::Init(window);

    // load resources, initialize the OpenGL states, ...

    sf::Clock deltaClock;

    audio_player audio;

    while (window.isOpen())
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::Resized) {
                // adjust the viewport when the window is resized
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }       

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        printf("x: %f y: %f \n", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        
        window.clear();

        //do rendering here


        ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}