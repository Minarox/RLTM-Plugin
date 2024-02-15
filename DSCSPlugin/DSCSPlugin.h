#pragma once
#pragma comment (lib, "pluginsdk.lib")
#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "crypt32.lib")

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// The structure of a ticker stat event
struct StatTickerParams {
    uintptr_t Receiver; // person who got a stat
    uintptr_t Victim; // person who is victim of a stat (only exists for demos afaik)
    uintptr_t StatEvent; // wrapper for the stat event
};

class DSCSPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

private:
	ix::WebSocket webSocket;
	bool game_in_progress = false;
	bool playback_in_progress = false;
	std::time_t match_started_at = std::time(0);

	void LoadWebSocket();
	void SetSpectatorUI(int sleep);
	void RemoveStatGraph();
	void Log(std::string message);
};
