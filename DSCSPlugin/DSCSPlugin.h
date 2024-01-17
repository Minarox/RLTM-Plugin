#pragma once
#pragma comment( lib, "pluginsdk.lib" )

#include "bakkesmod/plugin/bakkesmodplugin.h"

class DSCSPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

	void JoinSpectator();
	void SetSpectatorUI(int sleep);
	void RemoveStatGraph();
	void ShowHUD();
	void HideHUD();
	void MatchEnded();

private:
	bool matchInProgress = false;
	void Log(std::string msg);
};

