#include "pch.h"
#include "WSC.h"
#include "Controller/EventController.h"
#include "Controller/WebSocketController.h"
#include "Controller/GameDataController.h"
#include "Controller/GameHUDController.h"

BAKKESMOD_PLUGIN(WSC, "WebSocket Controller", plugin_version, PLUGINTYPE_SPECTATOR)

shared_ptr<CVarManagerWrapper> _globalCvarManager;

void WSC::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("wsc_url", "wss://example.com", "URL of the WebSocket server", false);

	ix::initNetSystem();
	SetSpectatorUI(100);
	SetStatGraph();
	SetSpectator();
	HookEvents();
	GetMatchData("onLoad");
	InitSocket();

	cvarManager->log("WSC plugin loaded.");
}

void WSC::onUnload()
{
	UnhookEvents();
	ResetDatas();
	socket.stop();
	ix::uninitNetSystem();

	cvarManager->log("WSC plugin unloaded.");
}
