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
#include <SFML/Network.hpp>

#include "audio_player.hpp"
#include "ggponet.h"

int main()
{
    //entt::registry registry;  //using entt
    //glm::vec3(0.0f, 0.0f, 0.0f); //using glm

    sf::RenderWindow window(sf::VideoMode(640, 480), "Fighting Game");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(false);
    window.setActive(true);
    ImGui::SFML::Init(window);

    //networking
    sf::UdpSocket socket;
    unsigned int port = 7000;
    const unsigned int data_size = 100;
    char data[data_size] = "Hello World";
    sf::IpAddress recipient = "localhost";

    //ui
    sf::Clock deltaClock;

    //audio
    audio_player audio;

    while (window.isOpen())
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::Resized)
                glViewport(0, 0, event.size.width, event.size.height);
        }

        //IO
        // printf("x: %f y: %f \n", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

        ImGui::SFML::Update(window, deltaClock.restart());

        window.clear();

        //        while (!rendering_paused)
        //        {
        //            //networking
        //            MSG msg = { 0 };
        //            int start, next, now;
        //
        //#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        //
        //            //https://docs.microsoft.com/en-us/windows/win32/api/timeapi/nf-timeapi-timegettime
        //            //System time in milliseconds since windows was started
        //            start = next = now = timeGetTime();
        //            printf("start: %i next: %i now: %i \n", start, next, now);
        //#elif
        //            printf("error - currently only builds on windows :( ");
        //#endif
        //            //while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        //            //    TranslateMessage(&msg);
        //            //    DispatchMessage(&msg);
        //            //    if (msg.message == WM_QUIT) {
        //            //        return 0;
        //            //    }
        //            //}
        //            //now = timeGetTime();
        //            ////VectorWar_Idle(max(0, next - now - 1));
        //            //if (now >= next) {
        //            //    //VectorWar_RunFrame(hwnd);
        //            //    next = now + (1000 / 60);
        //            //    printf("running frame \n");
        //            //}
        //
        //            rendering_paused = true;
        //        }

        window.display();

        ImGui::Begin("Networking UI");
        if (ImGui::Button("Send packet to localhost"))
        {
            if (socket.send(data, data_size, recipient, port) != sf::Socket::Done)
            {
                printf("error sending data to: %s", recipient);
            }
        }
        ImGui::End();

        ImGui::SFML::Render(window);
    }

    return 0;
}