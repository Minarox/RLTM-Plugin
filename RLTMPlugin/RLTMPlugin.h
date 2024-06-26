#pragma once
#pragma comment (lib, "pluginsdk.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "crypt32.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct StatTickerParams {
    uintptr_t Receiver;
    uintptr_t Victim;
    uintptr_t StatEvent;
};

class RLTMPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

private:
	ix::WebSocket webSocket;
	bool game_in_progress = false;
	bool overtime_in_progress = false;
	bool playback_in_progress = false;
	int game_time = 0;
	int total_game_time = 0;

	void LoadWebSocket();
	void SetSpectatorUI(int sleep);
	void RemoveStatGraph();
	void SetReplayAutoSave(bool status);
	void SetReady();
	bool CheckValidGame();
	void Log(std::string message);
};
