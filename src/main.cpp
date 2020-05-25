// main.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
//#include <assert.h>
#include <stdio.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/vec3.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <entt/entt.hpp>

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

#include "networking/networking_common.hpp"
#include "networking/network_settings.hpp"
#include "networking/chat_client.hpp"
#include "networking/chat_server.hpp"

std::thread* networking_thread;

void processEvents(sf::RenderWindow& window)
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
}

//Called X ticks per second
void tick(sf::Time deltaTime)
{
    //printf("tick: %f \n", deltaTime.asSeconds());
}

//called as fast as possible
void render(sf::RenderWindow& window, sf::Time deltaTime, network_settings& settings)
{
    ImGui::Begin("Some render UI");

    if(networking_thread == nullptr)
    {
        if (ImGui::Button("START AS SERVER"))
        {
            printf("starting as server");
            settings.is_server = true;

            networking_thread = new std::thread([&]()
            {
                settings.server.Run((uint16)settings.port);
            });
        };

        if (ImGui::Button("START AS CLIENT"))
        {
            printf("starting as client");
            settings.is_client = true;

            networking_thread = new std::thread([&]()
            {
                settings.client.Run( settings.addrServer );
            });
        };
    } else
    {
        ImGui::Text("You are a server or client");        
    }

    ImGui::End();
}

void PrintUsageAndExit(int rc = 1)
{
	fflush(stderr);
	printf(
R"usage(Usage:
    example_chat client SERVER_ADDR
    example_chat server [--port PORT]
)usage"
);
	fflush(stdout);
	exit(rc);
}

int main()
{
    //entt::registry registry;  //using entt
    //glm::vec3(0.0f, 0.0f, 0.0f); //using glm

    sf::RenderWindow window(sf::VideoMode(640, 480), "Fighting Game");
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(false);
    window.setActive(true);
    ImGui::SFML::Init(window);

    //ui
    sf::Clock deltaClock;

    //audio
    audio_player audio;

    //deltatime
    const sf::Time timePerFrame = sf::seconds(1.f / 60.f);
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    //networking
    network_settings net_set;
    net_set.addrServer.Clear();
    net_set.addrServer.ParseString("127.0.0.1");
    net_set.addrServer.m_port = net_set.port;

    InitSteamDatagramConnectionSockets();

    //Starts a thread to take in user input
    LocalUserInput_Init();

    while (window.isOpen())
    {
        sf::Time deltaTime = deltaClock.restart();
        timeSinceLastUpdate += deltaTime;

        ImGui::SFML::Update(window, deltaTime);
        window.clear();

        while (timeSinceLastUpdate > timePerFrame)
        {
            timeSinceLastUpdate -= timePerFrame;

            processEvents(window);
            tick(timePerFrame);
        }

        render(window, deltaTime, net_set);

        ImGui::SFML::Render(window);
        window.display();

        //IO
        // printf("x: %f y: %f \n", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    }

    ImGui::SFML::Shutdown();

    ShutdownSteamDatagramConnectionSockets();

    NukeProcess(0);
}

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