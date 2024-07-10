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

enum Event
{
	PLAYERS,
	MATCH,
	STATISTICS,
	STATISTIC,
	ENTITIES
};

std::map<Event, std::string> eventToTopic = {
	{ MATCH, "match" },
	{ STATISTIC, "statistic" },
	{ ENTITIES, "entities" }
};

struct StatTickerParams
{
	uintptr_t Receiver;
	uintptr_t Victim;
	uintptr_t StatEvent;
};

struct StatEventParams
{
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
	void SendSocketMessage(Event event, json payload);

	// Game data
	ServerWrapper GetServerWrapper();
	void GetMatchData(std::string caller);
	std::array<int, 2> GetScore(ServerWrapper server);
	json GetStatistics(ServerWrapper server);
	void OnStatTickerMessage(ServerWrapper server, void* params, std::string caller);
	void OnStatEvent(ServerWrapper server, void* params, std::string caller);
	void GetPlayerStatData(PriWrapper player, StatEventWrapper event, std::string caller);
	//void GetEntitiesData();
	void ResetDatas();

	// Game HUD
	void SetSpectatorUI(int sleep);
	void SetStatGraph();
	void SetReady();
};
