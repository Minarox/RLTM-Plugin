#pragma once
#pragma comment (lib, "pluginsdk.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "crypt32.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/wrappers/GameObject/Stats/StatEventWrapper.h"

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

#include <thread>
#include <chrono>

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

using json = nlohmann::json;
using namespace std;

enum Event
{
	MATCH,
	STATISTICS,
	STATISTIC,
	ENTITIES
};

map<Event, string> eventToTopic = {
	{ MATCH, "match" },
	{ STATISTICS, "statistics" },
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
	void HookEvents();
	void UnhookEvents();

	// WebSocket
	ix::WebSocket socket;
	json oldData;
	void InitSocket();
	void SendSocketMessage(Event event, json payload);

	// Game data
	string tickBuffer;
	bool isReplay = false;
	bool threadRunning = false;
	json entitiesData;
	ServerWrapper GetServerWrapper();
	void SetReplayState(bool state, string caller);
	void GetMatchData(string caller);
	array<int, 2> GetScore(ServerWrapper server);
	void GetStatisticsData(ServerWrapper server);
	void OnStatTickerMessage(ServerWrapper server, void* params);
	void OnStatEvent(ServerWrapper server, void* params);
	void GetPlayerStatData(PriWrapper player, StatEventWrapper event, ServerWrapper server);
	void GetEntitiesData();
	void SendEntitiesData();
	void ResetDatas();

	// Game Replays
	bool autoSaveReplay = false;
	void SetReplayAutoSave(bool status);

	// Game HUD
	void SetSpectatorUI(int sleep);
	void SetStatGraph();
	void SetReady();
};
