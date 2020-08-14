//#pragma once
//
//#include "networking_common.hpp"
//
///////////////////////////////////////////////////////////////////////////////
////
//// ChatClient
////
///////////////////////////////////////////////////////////////////////////////
//
//class ChatClient : private ISteamNetworkingSocketsCallbacks
//{
//public:
//
//	void Run(const SteamNetworkingIPAddr& serverAddr);
//
//private:
//
//	HSteamNetConnection m_hConnection;
//	ISteamNetworkingSockets* m_pInterface;
//
//	void PollIncomingMessages();
//
//	void PollConnectionStateChanges();
//
//	void PollLocalUserInput();
//
//	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) override;
//};
