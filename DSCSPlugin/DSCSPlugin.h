#pragma once
#pragma comment( lib, "pluginsdk.lib" )

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include <sio_client.h>

class DSCSPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

	void JoinSpectator();
	void SetSpectatorUI(int sleep);
	void RemoveStatGraph();
	void FetchStats();
	void SetReplayAutoSave(bool status);
	void ReadyUp();

private:
	sio::client socket;
	bool matchStatus = false;
	bool playbackStatus = false;
	bool highlightStatus = false;

	bool IsGameValid();
	void UpdateMatchStatus(bool status);
	void UpdatePlaybackStatus(bool status);
	void UpdateHighlightStatus(bool status);
	void ResetStatus();
	void Log(std::string msg);
};

