#include "system.hpp"

#include "modules/networking/components.hpp"
#include "modules/networking/helpers/client.hpp"
#include "modules/networking/helpers/general.hpp"
#include "modules/networking/helpers/server.hpp"
#include "modules/ui_networking/components.hpp"

void
game2d::init_networking_system(entt::registry& r)
{
  init_steam_datagram_connection_sockets();
};

void
game2d::update_networking_system(entt::registry& r, uint64_t milliseconds_dt)
{
  {
    auto& ui = r.ctx().at<SINGLETON_NetworkingUIComponent>();
    if (ui.start_server) {
      int port = 27020;
      start_server_or_quit(r, port);
      ui.start_server = false;
    }

    if (ui.start_client) {
      static std::string localhost = "127.0.0.1:27020";
      start_client(r, localhost);
      ui.start_client = false;
    }

    if (ui.close_networking) {
      close_networking();
      ui.close_networking = false;
    }
  }

  if (r.ctx().contains<SINGLETON_ServerComponent>())
    tick_server(r, milliseconds_dt);

  if (r.ctx().contains<SINGLETON_ClientComponent>())
    tick_client(r, milliseconds_dt);
};