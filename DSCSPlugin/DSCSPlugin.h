#pragma once
#pragma comment( lib, "pluginsdk.lib" )

#include "bakkesmod/plugin/bakkesmodplugin.h"

class DSCSPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	virtual void onLoad();
	virtual void onUnload();

private:
	void Log(std::string message);
};

