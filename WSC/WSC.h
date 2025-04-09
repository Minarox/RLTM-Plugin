#pragma once
#pragma comment (lib, "pluginsdk.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "crypt32.lib")

#include <thread>
#include <chrono>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/wrappers/GameObject/Stats/StatEventWrapper.h"

#include "GuiBase.h"
#include "PersistentStorage.h"
#include "version.h"

using json = nlohmann::json;
using namespace std;

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

// enum Event
// {
// 	MATCH,
// 	STATISTIC,
// 	ENTITIES,
// 	PLAYERS
// };

// map<Event, string> eventToTopic = {
// 	{ MATCH, "match" },
// 	{ STATISTIC, "statistic" },
// 	{ ENTITIES, "entities" },
// 	{ PLAYERS, "players" }
// };

// struct StatTickerParams
// {
// 	uintptr_t Receiver;
// 	uintptr_t Victim;
// 	uintptr_t StatEvent;
// };

// struct StatEventParams
// {
// 	uintptr_t PRI;
// 	uintptr_t StatEvent;
// };

class WSC: public BakkesMod::Plugin::BakkesModPlugin, public SettingsWindowBase
{
	// Boilerplate
	void onLoad() override;
	void onUnload() override;

	// Window settings
	void RenderSettings() override;

	// Cvars, notifiers and events
	void registerCvars();
	void registerNotifiers();
	void registerEvents();
	void unregisterEvents();

	// WebSocket
	ix::WebSocket socket;
	json dataBuffer;
	void setWSCallbacks();

	// Game data
	// string tickBuffer = "";
	// bool isReplay = false;
	// bool threadRunning = false;
	// json entitiesData = json::object();
	// ServerWrapper GetServerWrapper();
	// void SetReplayState(bool state, string caller);
	// void GetMatchData(string caller);
	// json GetScore(ServerWrapper server);
	// json GetStatistics(ServerWrapper server);
	// void GetPlayerStatData(ServerWrapper _server, void* params);
	// void GetEntitiesData();
	// void SendEntitiesData();
	// void GetPlayersData(ServerWrapper server);
	// void ResetDatas();

	// Game Replays
	// bool autoSaveReplay = false;
	// void SetReplayAutoSave(bool status);

	// Game HUD
	// void SetSpectatorUI(int sleep);
	// void SetStatGraph();
	// void SetReady();
	// void SetSpectator();
};
