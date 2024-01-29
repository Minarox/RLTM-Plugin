#pragma once
#pragma comment( lib, "pluginsdk.lib" )

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <sio_client.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DSCSPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

	void JoinSpectator();
	void SetSpectatorUI(int sleep);
	void RemoveStatGraph();
	void FetchStats();
	void FetchPlayers();
	void SetReplayAutoSave(bool status);
	void ReadyUp();

private:
	sio::client socket;
	std::time_t match_started_at = std::time(0);
	std::time_t match_ended_at = std::time(0);
	bool matchStatus = false;
	bool playbackStatus = false;
	bool highlightStatus = false;

	bool IsGameValid();
	void UpdateMatchStatus(bool status);
	void UpdatePlaybackStatus(bool status);
	void UpdateHighlightStatus(bool status);
	void ResetStatus();
	void Log(std::string message);
};

