#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/gns_networking/helpers.hpp"
#include "modules/gns_networking_client/helpers.hpp"
#include "modules/gns_networking_server/helpers.hpp"
#include "modules/gns_ui_networking/components.hpp"

void
game2d::init_networking_system(entt::registry& r)
{
  init_steam_datagram_connection_sockets();
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
    start_client(r, ui.host);
    ui.start_client = false;
    ui.client_was_started = true;
  }

  if (ui.close_networking) {
    close_networking();
    ui.close_networking = false;
  }

  if (ui.server_was_started)
    tick_server(r, milliseconds_dt);

  if (ui.client_was_started)
    tick_client(r, milliseconds_dt);
};