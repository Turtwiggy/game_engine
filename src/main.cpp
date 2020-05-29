// main.cpp : This file contains the 'main' function. Program execution begins and ends there.

//system stuff
#include <iostream>
#include <stdio.h>

//peoples stuff
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

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

//my stuff
#include "audio/audio_player.hpp"
#include "networking/networking_common.hpp"
#include "networking/network_settings.hpp"
#include "networking/chat_client.hpp"
#include "networking/chat_server.hpp"
#include "physics/physics_example.hpp"
#include "logo.hpp"

//-- start bad global things --
std::thread* networking_thread;
struct game_state
{
    physics_simulation physics;
};
int frame = 0;
//-- end bad global things, begin bad code --

void advance(game_state& state, sf::Time deltaTime)
{
    frame += 1;
    printf("ticking forward one physics iteration, new frame: %i ", frame);

    state.physics.step_simulation(deltaTime.asMilliseconds());
}

void process_events(sf::RenderWindow& window, sf::Event& event, game_state& state, sf::Time deltaTime)
{
    while (window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            advance(state, deltaTime); //only advance physics if space is pressed

        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::Resized)
            glViewport(0, 0, event.size.width, event.size.height);
    }
}

//Called X ticks per second
void tick(game_state& state, sf::Time deltaTime, sf::Event& events)
{
    //update game state...
    //printf("ticking frame %i game state time: %f \n", frame, deltaTime.asSeconds());
}

void render_ui(sf::RenderWindow& window, game_state& state, network_settings& settings)
{
    ImGui::Begin("Networking UI");

    if (networking_thread == nullptr)
    {
        if (ImGui::Button("START AS SERVER"))
        {
            printf("starting as server");
            InitSteamDatagramConnectionSockets();
            settings.is_server = true;
            quit = false;

            networking_thread = new std::thread([&]()
                {
                    settings.server.Run((uint16)settings.port);
                });
        };

        if (ImGui::Button("START AS CLIENT"))
        {
            printf("starting as client");
            InitSteamDatagramConnectionSockets();
            settings.is_client = true;
            quit = false;

            networking_thread = new std::thread([&]()
                {
                    settings.client.Run(settings.addrServer);
                });
        };
    }
    else
    {
        ImGui::Text("You are a server or client");

        //ImGui::InputText();

        if (ImGui::Button("Quit"))
        {
            //stop networking
            quit = true;
            //networking_thread->join();
            ShutdownSteamDatagramConnectionSockets();

            delete networking_thread;
        }
    }

    ImGui::End();
}

//called as fast as possible
void render(sf::RenderWindow& window, game_state& state, network_settings& settings)
{




    //physics stuff
    //int num_objects = state.physics.dynamicsWorld->getNumCollisionObjects();
    ////printf("num_objects: %i", num_objects);

    ////print positions of all objects
    //for (int j = num_objects - 1; j >= 0; j--)
    //{
    //    btCollisionObject* obj = state.physics.dynamicsWorld->getCollisionObjectArray()[j];
    //    btRigidBody* body = btRigidBody::upcast(obj);
    //    btTransform trans;
    //    if (body && body->getMotionState())
    //    {
    //        body->getMotionState()->getWorldTransform(trans);
    //    }
    //    else
    //    {
    //        trans = obj->getWorldTransform();
    //    }
    //    //printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
    //}

    render_ui(window, state, settings);
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
    unsigned int m_width = 480;
    unsigned int m_height = 640;

    //library checks
    //entt::registry registry;  //using entt
    //glm::vec3(0.0f, 0.0f, 0.0f); //using glm
    //glEnable(GL_TEXTURE_2D); //using opengl

    sf::ContextSettings settings;
    settings.antialiasingLevel = 2;
    settings.majorVersion = 3;
    settings.minorVersion = 2;

    sf::RenderWindow window(sf::VideoMode(m_height, m_width), "Fighting Game", sf::Style::Default, settings);
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

    game_state state_previous;
    game_state state_current;
    state_current.physics.init_physics();

    //networking
    network_settings net_set;
    net_set.addrServer.Clear();
    net_set.addrServer.ParseString("127.0.0.1");
    net_set.addrServer.m_port = net_set.port;


    //rendering
    printf("init rendering");
    bgfx::Init init;
    init.type = bgfx::RendererType::Count;
    //init.vendorId = BGFX_PCI_ID_AMD;
    init.resolution.width = m_width;
    init.resolution.height = m_height;
    //init.resolution.reset = m_reset;
    bgfx::init(init);

    // Set view 0 clear state.
    bgfx::setViewClear(0
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 0x303030ff
        , 1.0f
        , 0
    );
    uint32_t m_debug = BGFX_DEBUG_TEXT;
    uint32_t m_reset = BGFX_RESET_VSYNC;
    // Enable debug text.
    bgfx::setDebug(m_debug);

    while (window.isOpen())
    {
        sf::Time deltaTime = deltaClock.restart();
        timeSinceLastUpdate += deltaTime;

        ImGui::SFML::Update(window, deltaTime);

        //e.g. if time is 1, we process 60 frames. CPU Spike much.
        while (timeSinceLastUpdate >= timePerFrame)
        {
            state_previous = state_current;

            // handle events
            sf::Event event;
            process_events(window, event, state_current, timePerFrame);
            tick(state_current, timePerFrame, event);

            timeSinceLastUpdate -= timePerFrame;
        }

        window.clear();

        const double alpha = timeSinceLastUpdate / timePerFrame;
        //lerp between game states
        //game_state new_state = state_current * alpha + state_previous * ( 1.0 - alpha );
        //render(window, new_state, net_set);
        render(window, state_current, net_set);

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx::touch(0);

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextImage(
            bx::max<uint16_t>(uint16_t(m_width / 2 / 8), 20) - 20
            , bx::max<uint16_t>(uint16_t(m_height / 2 / 16), 6) - 6
            , 40
            , 12
            , s_logo
            , 160
        );
        bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

        // Use debug font to print information about this example.
        bgfx::dbgTextClear();
        bgfx::dbgTextImage(
            bx::max<uint16_t>(uint16_t(m_width / 2 / 8), 20) - 20
            , bx::max<uint16_t>(uint16_t(m_height / 2 / 16), 6) - 6
            , 40
            , 12
            , s_logo
            , 160
        );
        bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

        bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
        bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

        const bgfx::Stats* stats = bgfx::getStats();
        bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
            , stats->width
            , stats->height
            , stats->textWidth
            , stats->textHeight
        );

        ImGui::SFML::Render(window);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();

        window.display();


        //IO
        // printf("x: %f y: %f \n", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    }

    ImGui::SFML::Shutdown();

    NukeProcess(0);
}