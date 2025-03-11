#include "pch.hpp"

#include "steam_helpers.hpp"

namespace game2d {

void
SteamAPIDebugTextHook(int nSeverity, const char* pchDebugText)
{
  // if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
  // if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
  SDL_Log("%s", pchDebugText);

  if (nSeverity >= 1) {
    // place to set a breakpoint for catching API errors
    int x = 3;
    (void)x;
  }
};

void
init_steam(entt::registry& r)
{
#if defined(_DEBUG)
  // load app id
  std::ifstream file("steam_appid.txt");
  if (!file) {
    SDL_Log("No steam_appid.txt detected");
    exit(1);
  }
  std::string line;
  std::getline(file, line);
  file.close();
  const int app_id = std::stoi(line);

  // Check if your executable was launched through Steam, and relaunches if it wasn't.
  // Note: if steam_app.txt is present, this will return false regardless.
  // This allows you to develop and text without launching the game through the steam client.
  // Make sure to remove steam_appid.txt file when uploading the game to steam depot.
  if (SteamAPI_RestartAppIfNecessary(app_id)) {
    SDL_Log("Restarting app");
    exit(1);
  }
#endif

  if (!SteamAPI_Init()) {
    SDL_Log("Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed).\n");
    exit(1);
  }

  // set our debug handler
  SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

  if (!SteamUser()->BLoggedOn()) {
    SDL_Log("Steam user is not logged in");
    exit(1);
  }

  char szCommandLine[1024] = {};
  SteamApps()->GetLaunchCommandLine(szCommandLine, sizeof(szCommandLine));
  SDL_Log("GetLaunchCommandLine: %s", szCommandLine);
}

} // namespace game2d