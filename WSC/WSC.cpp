#include "pch.h"
#include "WSC.h"
// #include "Controller/EventController.h"
// #include "Controller/WebSocketController.h"
// #include "Controller/GameDataController.h"
// #include "Controller/GameHUDController.h"

BAKKESMOD_PLUGIN(WSC, "WebSocket Controller", plugin_version, PLUGINTYPE_SPECTATOR)

shared_ptr<CVarManagerWrapper> _globalCvarManager;
shared_ptr<PersistentStorage> _persistentStorage;

void WSC::onLoad()
{
	_globalCvarManager = cvarManager;

	// Required for ixwebsocket
	ix::initNetSystem();

	setWSCallbacks();

	registerCvars();
	registerNotifiers();
	registerEvents();

	// SetSpectatorUI(100);
	// SetStatGraph();
	// SetSpectator();
	// GetMatchData("onLoad");
}

void WSC::onUnload()
{
	socket.stop();
	unregisterEvents();
	// ResetDatas();

	// Disable auto save replays
	cvarManager->executeCommand("ranked_autosavereplay_all 0", false);

	// Required for ixwebsocket
	ix::uninitNetSystem();
}

void WSC::registerCvars()
{
	_persistentStorage = make_shared<PersistentStorage>(this, "wsc", true, true);

	_persistentStorage->RegisterPersistentCvar("wsc_auto_connect", "0", "Auto connect to server when plugin loaded", true, true, 0, true, 1);

	_persistentStorage->RegisterPersistentCvar("wsc_auto_reconnect", "1", "Auto reconnect to server", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getBoolValue()) socket.enableAutomaticReconnection();
			else socket.disableAutomaticReconnection();
		});

	_persistentStorage->RegisterPersistentCvar("wsc_auto_save_replays", "1", "Auto reconnect to server", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			cvarManager->executeCommand("ranked_autosavereplay_all " + cvar.getStringValue(), false);
		});

	_persistentStorage->RegisterPersistentCvar("wsc_handshake_timeout", "3", "Handshake timeout", true, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			socket.setHandshakeTimeout(cvar.getIntValue());
		});

	_persistentStorage->RegisterPersistentCvar("wsc_max_wait_between_retries", "500", "Maximum wait between reconnection retries", true, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			socket.setMaxWaitBetweenReconnectionRetries(cvar.getIntValue());
		});

	_persistentStorage->RegisterPersistentCvar("wsc_min_wait_between_retries", "500", "Minimum wait between reconnection retries", true, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			socket.setMinWaitBetweenReconnectionRetries(cvar.getIntValue());
		});

	_persistentStorage->RegisterPersistentCvar("wsc_per_message_deflate", "1", "Per message deflate", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getBoolValue()) socket.enablePerMessageDeflate();
			else socket.disablePerMessageDeflate();
		});

	_persistentStorage->RegisterPersistentCvar("wsc_ping_interval", "1", "Ping interval", true, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			socket.setPingInterval(cvar.getIntValue());
		});

	_persistentStorage->RegisterPersistentCvar("wsc_url", "ws://localhost:3000", "URL of the WebSocket server", true)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			socket.setUrl(cvar.getStringValue());

			CVarWrapper autoConnect = cvarManager->getCvar("wsc_auto_connect");
			if (autoConnect.getBoolValue())
			{
				if (socket.getReadyState() != ix::ReadyState::Closed) socket.stop();
				if (cvar.getStringValue() != "") socket.start();
				else LOG("Can't connect to the server: WebSocket URL is empty");
			}
		});
}

void WSC::registerNotifiers()
{
	cvarManager->registerNotifier("wsc_version", [this](std::vector<std::string> args) {
		LOG("WSC plugin version: " + string(plugin_version));
	}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("wsc_connect", [this](std::vector<std::string> args) {
		if (socket.getReadyState() != ix::ReadyState::Open) socket.start();
	}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("wsc_disconnect", [this](std::vector<std::string> args) {
		if (socket.getReadyState() != ix::ReadyState::Closed) socket.stop();
	}, "", PERMISSION_ALL);
}


void WSC::registerEvents()
{
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
}

void WSC::unregisterEvents()
{
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
}

void WSC::setWSCallbacks()
{
	socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
	{
		switch (msg->type)
		{
			case ix::WebSocketMessageType::Open:
				LOG("WebSocket connection opened");
				break;

			case ix::WebSocketMessageType::Message:
				LOG("WebSocket message received: " + msg->str);
				break;

			case ix::WebSocketMessageType::Close:
				LOG("WebSocket connection closed");
				break;

			case ix::WebSocketMessageType::Error:
				LOG("WebSocket error: " + msg->errorInfo.reason);
				break;

			case ix::WebSocketMessageType::Ping:
				LOG("WebSocket ping");
				break;

			case ix::WebSocketMessageType::Pong:
				LOG("WebSocket pong");
				break;
		}
	});
}

void WSC::RenderSettings()
{
    ImGui::TextUnformatted("WebSocket Controller plugin");
	//ImGui::TextDisabled("v" + to_string(plugin_version).c_str());
    ImGui::TextUnformatted("Author: @Minarox");

	ImGui::Separator();

	// ImGui::SeparatorText("WebSocket Configuration");
	CVarWrapper autoConnect = cvarManager->getCvar("wsc_auto_connect");
	if (!autoConnect) { return; }
	bool enabled = autoConnect.getBoolValue();
	if (ImGui::Checkbox("Auto connect to server when loaded", &enabled)) {
		autoConnect.setValue(enabled);
	}

	CVarWrapper url = cvarManager->getCvar("wsc_url");
	if (!url) { return; }
	//static char urlInput[512] = to_string(url.getStringValue()).c_str();
	//ImGui::InputText("WebSocket URL", urlInput, IM_ARRAYSIZE(urlInput));
	ImGui::SameLine();
	// TODO: Get socket state
	if (ImGui::Button("Connect")) {
		gameWrapper->Execute([this](GameWrapper* gw) {
			cvarManager->executeCommand("wsc_connect");
		});
	}

	ImGui::TextUnformatted("State: ");
	ImGui::SameLine();

	// TODO: Get socket state
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 0.0f), "Disconnected");
	// ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting");
	// ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Reconnecting");
	// ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 0.0f), "Connected");

	ImGui::SameLine();
	// TODO: If lastSocketStateUpdate
	ImGui::TextDisabled(" since ..."); // TODO: Show lastSocketStateUpdate date

	if (ImGui::TreeNode("More WebSocket settings"))
	{
		// TODO: Add other socket settings
		// wsc_auto_reconnect (checkbox)
		// wsc_per_message_deflate (checkbox)
		// wsc_handshake_timeout (integer input)
		// wsc_max_wait_between_retries (integer input)
		// wsc_min_wait_between_retries (integer input)
		// wsc_ping_interval (integer input)
		ImGui::TreePop();
	}
}