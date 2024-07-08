#pragma once
#pragma comment (lib, "pluginsdk.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "crypt32.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

using json = nlohmann::json;

struct StatTickerParams {
	uintptr_t Receiver;
	uintptr_t Victim;
	uintptr_t StatEvent;
};

struct StatEventParams {
    uintptr_t PRI;
    uintptr_t StatEvent;
};

class RLTM: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*/
{
	// Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	// Hooks
	bool isHooked = false;
	void HookEvents();
	void UnhookEvents();

	// WebSocket
	ix::WebSocket socket;
	json oldData;
	void InitSocket();
	void SendSocketMessage(std::string topic, json payload);

	// Game datas
	ServerWrapper GetServerWrapper();
	void GetPlayersData(std::string caller);
	void GetMatchData(std::string caller);
	std::array<int, 2> GetGameScore(ServerWrapper server);
	json GetGameStatistics(ServerWrapper server);
	void GetStatEventData(ServerWrapper server, void* params, std::string caller);
	void ResetDatas();

	// Game HUD
	void SetSpectatorUI(int sleep);
	void SetStatGraph();
	void SetReady();
};
