#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/gns_networking/helpers.hpp"
#include "modules/gns_networking_client/helpers.hpp"
#include "modules/gns_networking_server/helpers.hpp"
#include "modules/gns_ui_networking/components.hpp"

#include <iostream>
#include <queue>
#include <thread>

// input via terminal
std::thread* s_pThreadUserInput = nullptr;
std::mutex mutexUserInputQueue;
std::queue<std::string> queueUserInput;

// trim from start (in place)
static inline void
ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void
rtrim(std::string& s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
}

void
LocalUserInput_Init()
{
  s_pThreadUserInput = new std::thread([]() {
    while (true) { // should be app.running
      char szLine[4000];
      if (!fgets(szLine, sizeof(szLine), stdin)) {
        // Well, you would hope that you could close the handle
        // from the other thread to trigger this.  Nope.
        break;
      }

      mutexUserInputQueue.lock();
      queueUserInput.push(std::string(szLine));
      mutexUserInputQueue.unlock();
    }
  });
}

// Read the next line of input from stdin, if anything is available.
bool
LocalUserInput_GetNext(std::string& result)
{
  bool got_input = false;
  mutexUserInputQueue.lock();
  while (!queueUserInput.empty() && !got_input) {
    result = queueUserInput.front();
    queueUserInput.pop();
    ltrim(result);
    rtrim(result);
    got_input = !result.empty(); // ignore blank lines
  }
  mutexUserInputQueue.unlock();
  return got_input;
}

void
game2d::init_networking_system(entt::registry& r)
{
  init_steam_datagram_connection_sockets();
  LocalUserInput_Init();
};

void
game2d::update_networking_system(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& ui = get_first_component<SINGLETON_NetworkingUIComponent>(r);

  if (ui.start_server) {
    start_server_or_quit(r, ui.server_port);
    ui.start_server = false;
    ui.server_was_started = true;
  }

  if (ui.start_client) {
    start_client(r, ui.host, ui.server_port);
    ui.start_client = false;
    ui.client_was_started = true;
  }

  if (ui.close_networking) {
    close_networking();
    ui.close_networking = false;
  }

  if (ui.server_was_started) {
    tick_server(r, milliseconds_dt);

    std::string result;
    LocalUserInput_GetNext(result);
    if (!result.empty()) {
      auto& server = get_first_component<SINGLETON_ServerComponent>(r);
      send_string_to_all_clients(server, result);
    }
  }

  if (ui.client_was_started)
    tick_client(r, milliseconds_dt);
};