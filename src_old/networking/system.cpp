#include "system.hpp"

#include "networking/components.hpp"
#include "networking/helpers/client.hpp"
#include "networking/helpers/general.hpp"
#include "networking/helpers/server.hpp"
#include "ui_networking/components.hpp"

void
game2d::init_networking_system(const GameEditor& editor)
{
  init_steam_datagram_connection_sockets();
};

void
game2d::update_networking_system(GameEditor& editor, Game& game, uint64_t milliseconds_dt)
{
  auto& ui = editor.networking_ui;
  if (ui.start_server) {
    int port = 27020;
    start_server_or_quit(editor, game, port);
    ui.start_server = false;
    ui.server_was_started = true;
  }

  if (ui.start_client) {
    static std::string localhost = "127.0.0.1:27020";
    start_client(editor, game, localhost);
    ui.start_client = false;
    ui.client_was_started = true;
  }

  if (ui.close_networking) {
    close_networking();
    ui.close_networking = false;
  }

  if (ui.server_was_started)
    tick_server(editor, game, milliseconds_dt);

  if (ui.client_was_started)
    tick_client(editor, game, milliseconds_dt);
};