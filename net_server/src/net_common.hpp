#pragma once

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>

// other lib headers
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#ifdef WIN32
#include <windows.h> // Ug, for NukeProcess -- see below
#else
#include <signal.h>
#include <unistd.h>
#endif

namespace net_server {

// ---- GLOBALS
bool g_bQuit = false;
SteamNetworkingMicroseconds g_logTimeZero;
// ------------

// We do this because I won't want to figure out how to cleanly shut
// down the thread that is reading from stdin.
static void
NukeProcess(int rc)
{
#ifdef WIN32
  ExitProcess(rc);
#else
  (void)rc; // Unused formal parameter
  kill(getpid(), SIGKILL);
#endif
}

static void
DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
  SteamNetworkingMicroseconds time =
    SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
  printf("%10.6f %s\n", time * 1e-6, pszMsg);
  fflush(stdout);
  if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
    fflush(stdout);
    fflush(stderr);
    NukeProcess(1);
  }
}

static void
FatalError(const char* fmt, ...)
{
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char* nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

static void
Printf(const char* fmt, ...)
{
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char* nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

static void
InitSteamDatagramConnectionSockets()
{
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg))
    FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
#else
  SteamDatagramClient_SetAppID(570); // Just set something, doesn't matter what
  // SteamDatagramClient_SetUniverse( k_EUniverseDev );

  SteamDatagramErrMsg errMsg;
  if (!SteamDatagramClient_Init(true, errMsg))
    FatalError("SteamDatagramClient_Init failed.  %s", errMsg);

  // Disable authentication when running with Steam, for this
  // example, since we're not a real app.
  //
  // Authentication is disabled automatically in the open-source
  // version since we don't have a trusted third party to issue
  // certs.
  SteamNetworkingUtils()->SetGlobalConfigValueInt32(
    k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif

  g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

  SteamNetworkingUtils()->SetDebugOutputFunction(
    k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

static void
ShutdownSteamDatagramConnectionSockets()
{
  // Give connections time to finish up.  This is an application layer protocol
  // here, it's not TCP.  Note that if you have an application and you need to be
  // more sure about cleanup, you won't be able to do this.  You will need to send
  // a message and then either wait for the peer to close the connection, or
  // you can pool the connection to see if any reliable data is pending.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  GameNetworkingSockets_Kill();
#else
  SteamDatagramClient_Kill();
#endif
}

// ---- Non-blocking console user input.

// ---- GLOBALS
std::mutex mutexUserInputQueue;
std::queue<std::string> queueUserInput;

std::thread* s_pThreadUserInput = nullptr;
// ------------

void
LocalUserInput_Init()
{
  s_pThreadUserInput = new std::thread([]() {
    while (!g_bQuit) {
      char szLine[4000];
      if (!fgets(szLine, sizeof(szLine), stdin)) {
        // Well, you would hope that you could close the handle
        // from the other thread to trigger this.  Nope.
        if (g_bQuit)
          return;
        g_bQuit = true;
        Printf("Failed to read on stdin, quitting\n");
        break;
      }

      mutexUserInputQueue.lock();
      queueUserInput.push(std::string(szLine));
      mutexUserInputQueue.unlock();
    }
  });
}

// trim from start (in place)
static inline void
ltrim(std::string& s)
{
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void
rtrim(std::string& s)
{
  s.erase(
    std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(),
    s.end());
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

} // namespace net_server