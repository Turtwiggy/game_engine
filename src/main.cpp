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

struct networking_settings
{    
    // Create a socket to receive a message from anyone
    sf::UdpSocket socket;

    unsigned short port = 7000;
};

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

void runUdpServer()
{
    networking_settings settings;

    if (settings.socket.bind(settings.port) != sf::Socket::Done)
        return;

    std::cout << "Server is listening to port " << settings.port << ", waiting for a message... " << std::endl;

    // Wait for a message
    char in[128];
    std::size_t received;
    sf::IpAddress sender;
    unsigned short senderPort;
    if (settings.socket.receive(in, sizeof(in), received, sender, senderPort) != sf::Socket::Done)
        return;
    std::cout << "Message received from client " << sender << ": \"" << in << "\"" << std::endl;

    // Send an answer to the client
    const char out[] = "Hi, I'm the server";
    if (settings.socket.send(out, sizeof(out), sender, senderPort) != sf::Socket::Done)
        return;
    std::cout << "Message sent to the client: \"" << out << "\"" << std::endl;
}

void runUdpClient()
{
    networking_settings settings;
      
    // Ask for the server address
    sf::IpAddress server;
    do
    {
        std::cout << "Type the address or name of the server to connect to: ";
        std::cin >> server;
    } while (server == sf::IpAddress::None);

    // Send a message to the server
    std::string message;

    do
    {
        std::cout << "Type a message: ";
        std::cin >> message;
    } while (message.empty());

    if (settings.socket.send(message.c_str(), sizeof(message), server, settings.port) != sf::Socket::Done)
        return;
    std::cout << "Message sent to the server: \"" << message << "\"" << std::endl;

    // Receive an answer from anyone (but most likely from the server)
    char in[128];
    std::size_t received;
    sf::IpAddress sender;
    unsigned short senderPort;
    if (settings.socket.receive(in, sizeof(in), received, sender, senderPort) != sf::Socket::Done)
        return;
    std::cout << "Message received from " << sender << ": \"" << in << "\"" << std::endl;
}

//Called X ticks per second
void tick(sf::Time deltaTime)
{
    //printf("tick: %f \n", deltaTime.asSeconds());
}

//called as fast as possible
void render(sf::RenderWindow& window, sf::Time deltaTime )
{
    ImGui::Begin("Some render UI");

    if (ImGui::Button("START AS SERVER"))
    {
        printf("starting as server");
        runUdpServer();
    };

    if (ImGui::Button("START AS CLIENT"))
    {
        printf("starting as client");
        runUdpClient();
    };

    ImGui::End();
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

        render(window, deltaTime);

        ImGui::SFML::Render(window);
        window.display();
        
        //IO
        // printf("x: %f y: %f \n", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    }

    ImGui::SFML::Shutdown();
    return 0;
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