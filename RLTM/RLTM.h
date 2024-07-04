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

class RLTM: public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*/
{
	// Boilerplate
	virtual void onLoad();
	virtual void onUnload();

	// Hooks
	bool hooked;
	void HookEvents();
	void UnhookEvents();

	// WebSocket
	ix::WebSocket socket;
	void InitSocket();
	void SendSocketMessage(std::string topic, json message);

	// Game datas
	ServerWrapper GetServerWrapper();
	void FetchGameData(int isReplayingGoal);
	std::array<int, 2> GetGameScore(ServerWrapper server);

	// Game HUD
	void SetSpectatorUI(int sleep);
	void RemoveStatGraph();
	void SetReady()
};

