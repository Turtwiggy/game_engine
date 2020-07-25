//#pragma once
//
//#include <stdarg.h>
//#include <string.h>
//#include <iostream>
//#include <algorithm>
//#include <string>
//#include <random>
//#include <chrono>
//#include <thread>
//#include <mutex>
//#include <queue>
//#include <map>
//#include <cctype>
//#include <assert.h>
//
////gamenetworkingsockets
//#include <steam/steamnetworkingsockets.h>
//#include <steam/isteamnetworkingutils.h>
//
//#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
//#include <steam/steam_api.h>
//#endif
//
//#if defined(WIN32) || defined(WIN_32) || defined(__WIN32__)
//	// Ug, for NukeProcess -- see below
//	#include <windows.h>
//#else
//	#include <unistd.h>
//	#include <signal.h>
//#endif
//
////////////////////////////////////////////////////////////////////////////////
////
//// Common stuff
////
//////////////////////////////////////////////////////////////////////////////
//
//static bool quit;
//static SteamNetworkingMicroseconds g_logTimeZero;
//static std::mutex mutexUserInputQueue;
//static std::queue< std::string > queueUserInput;
//static std::thread* s_pThreadUserInput = nullptr;
//
////https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/examples/example_chat.cpp
//// We do this because I won't want to figure out how to cleanly shut
//// down the thread that is reading from stdin.
//static void NukeProcess(int rc)
//{
//#ifdef WIN32
//	ExitProcess(rc);
//#else
//	kill(getpid(), SIGKILL);
//#endif
//}
//
//static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
//{
//	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
//	printf("%10.6f %s\n", time * 1e-6, pszMsg);
//	fflush(stdout);
//	if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
//	{
//		fflush(stdout);
//		fflush(stderr);
//		NukeProcess(1);
//	}
//}
//
//static void FatalError(const char* fmt, ...)
//{
//	char text[2048];
//	va_list ap;
//	va_start(ap, fmt);
//	vsprintf(text, fmt, ap);
//	va_end(ap);
//	char* nl = strchr(text, '\0') - 1;
//	if (nl >= text && *nl == '\n')
//		*nl = '\0';
//	DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
//}
//
//static void Printf(const char* fmt, ...)
//{
//	char text[2048];
//	va_list ap;
//	va_start(ap, fmt);
//	vsprintf(text, fmt, ap);
//	va_end(ap);
//	char* nl = strchr(text, '\0') - 1;
//	if (nl >= text && *nl == '\n')
//		*nl = '\0';
//	DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
//}
//
//static void InitSteamDatagramConnectionSockets()
//{
//#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
//	SteamDatagramErrMsg errMsg;
//	if (!GameNetworkingSockets_Init(nullptr, errMsg))
//		FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
//#else
//	SteamDatagramClient_SetAppID(570); // Just set something, doesn't matter what
//	//SteamDatagramClient_SetUniverse( k_EUniverseDev );
//
//	SteamDatagramErrMsg errMsg;
//	if (!SteamDatagramClient_Init(true, errMsg))
//		FatalError("SteamDatagramClient_Init failed.  %s", errMsg);
//
//	// Disable authentication when running with Steam, for this
//	// example, since we're not a real app.
//	//
//	// Authentication is disabled automatically in the open-source
//	// version since we don't have a trusted third party to issue
//	// certs.
//	SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
//#endif
//
//	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
//
//	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
//}
//
//static void ShutdownSteamDatagramConnectionSockets()
//{
//	// Give connections time to finish up.  This is an application layer protocol
//	// here, it's not TCP.  Note that if you have an application and you need to be
//	// more sure about cleanup, you won't be able to do this.  You will need to send
//	// a message and then either wait for the peer to close the connection, or
//	// you can pool the connection to see if any reliable data is pending.
//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
//
//#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
//	GameNetworkingSockets_Kill();
//#else
//	SteamDatagramClient_Kill();
//#endif
//}
//
///////////////////////////////////////////////////////////////////////////////
////
//// Non-blocking console user input.  Sort of.
//// Why is this so hard?
////
///////////////////////////////////////////////////////////////////////////////
//
//// You really gotta wonder what kind of pedantic garbage was
//// going through the minds of people who designed std::string
//// that they decided not to include trim.
//// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
//
//// trim from start (in place)
//static inline void ltrim(std::string& s) {
//	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
//		return !std::isspace(ch);
//		}));
//}
//
//// trim from end (in place)
//static inline void rtrim(std::string& s) {
//	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
//		return !std::isspace(ch);
//		}).base(), s.end());
//}
//
//void LocalUserInput_Init();
//
//void LocalUserInput_Kill();
//
//// You really gotta wonder what kind of pedantic garbage was
//// going through the minds of people who designed std::string
//// that they decided not to include trim.
//// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
//
//// Read the next line of input from stdin, if anything is available.
//bool LocalUserInput_GetNext(std::string& result);
