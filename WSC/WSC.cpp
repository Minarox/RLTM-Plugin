#include "pch.h"
#include "WSC.h"
// #include "Controller/EventController.h"
// #include "Controller/WebSocketController.h"
// #include "Controller/GameDataController.h"
// #include "Controller/GameHUDController.h"

BAKKESMOD_PLUGIN(WSC, "WebSocket Controller", plugin_version, PLUGINTYPE_SPECTATOR)

shared_ptr<CVarManagerWrapper> _globalCvarManager;

void WSC::onLoad()
{
	_globalCvarManager = cvarManager;

	persistentStorage = std::make_shared<PersistentStorage>(this, "WSCStorage", true, true);

	// Required for ixwebsocket
	ix::initNetSystem();

	registerCvars();
	registerNotifiers();

	// SetSpectatorUI(100);
	// SetStatGraph();
	// SetSpectator();
	// HookEvents();
	// GetMatchData("onLoad");
	// InitSocket();

	cvarManager->log("WSC " + plugin_version + " loaded.");

	CVarWrapper wscUrlPersistent = cvarManager->getCvar("wsc_url_persistent");
	cvarManager->log("default wsc_url_persistent: " + wscUrlPersistent.getStringValue());
	CVarWrapper wscUrl = cvarManager->getCvar("wsc_url");
	cvarManager->log("default wsc_url: " + wscUrl.getStringValue());
}

void WSC::onUnload()
{
	// unhookEvents();
	// ResetDatas();
	// socket.stop();

	// Required for ixwebsocket
	ix::uninitNetSystem();

	cvarManager->log("WSC " + plugin_version + " unloaded.");
}

void WSC::registerCvars()
{
	persistentStorage->RegisterPersistentCvar("wsc_url_persistent", "wss://example.com", "URL of the WebSocket server", true)
		.OnPersistentCvarChanged([this](std::string oldValue, CVarWrapper cvar) {
			cvarManager->log("wsc_url_persistent changed: " + cvar.getStringValue());
		});

	cvarManager->registerCvar("wsc_url", "wss://example.com", "URL of the WebSocket server", true)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			cvarManager->log("wsc_url changed: " + cvar.getStringValue());
		});

	// cvarManager->registerCvar("wsc_autoconnect", "1", "Enable automatic connection to the WebSocket server when loading plugin", true, true, 0, true, 1);
	// cvarManager->registerCvar("wsc_autoreconnect", "1", "Enable automatic reconnection to the WebSocket server", true, true, 0, true, 1);
}

void WSC::registerNotifiers()
{
	cvarManager->registerNotifier("wsc_connect", [this](std::vector<std::string> args) {
		cvarManager->log("wsc_connect executed.");
	}, "", PERMISSION_ALL);
	cvarManager->registerNotifier("wsc_disconnect", [this](std::vector<std::string> args) {
		cvarManager->log("wsc_disconnect executed.");
	}, "", PERMISSION_ALL);
}


// void WSC::hookEvents(GameWrapper* gameWrapper, WSC* wscInstance)
// {
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.Team_TA.OnScoreUpdated", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function Engine.WorldInfo.EventPauseChanged", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
// 	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", bind(&WSC::SetReplayState, wscInstance, true, placeholders::_1));
// 	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", bind(&WSC::SetReplayState, wscInstance, false, placeholders::_1));
// 	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&WSC::GetPlayerStatData, wscInstance, placeholders::_1, placeholders::_2));
// 	gameWrapper->HookEventPost("Function Engine.GameViewportClient.Tick", bind(&WSC::GetEntitiesData, wscInstance));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&WSC::ResetDatas, wscInstance));
// 	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", bind(&WSC::SetSpectatorUI, wscInstance, 100));
// 	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", bind(&WSC::SetSpectatorUI, wscInstance, 0));
// 	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", bind(&WSC::SetStatGraph, wscInstance));
// 	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", bind(&WSC::SetReady, wscInstance));
// }

// void WSC::unhookEvents(GameWrapper* gameWrapper)
// {
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer");
// 	gameWrapper->UnhookEvent("Function TAGame.PRI_TA.OnTeamChanged");
// 	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit");
// 	gameWrapper->UnhookEvent("Function TAGame.Team_TA.OnScoreUpdated");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated");
// 	gameWrapper->UnhookEvent("Function Engine.WorldInfo.EventPauseChanged");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
// 	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
// 	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");
// 	gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");
// 	gameWrapper->UnhookEventPost("Function Engine.GameViewportClient.Tick");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");
// 	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
// 	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");
// 	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");
// 	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");
// }