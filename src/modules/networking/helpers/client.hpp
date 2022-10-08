#pragma once

#include "game/components/app.hpp"
#include "modules/networking/components.hpp"

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <string>
#include <vector>

namespace game2d {

void
start_client(GameEditor& editor, Game& game, const std::string& addr);

void
tick_client(GameEditor& editor, Game& game, uint64_t milliseconds_dt);

void
client_receive_messages_on_connection(SINGLETON_ClientComponent& client, std::vector<std::string>& result);

void
client_poll_connections(SINGLETON_ClientComponent& client);

} // namespace game2d