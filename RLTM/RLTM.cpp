#include "pch.h"
#include "RLTM.h"


BAKKESMOD_PLUGIN(RLTM, "Rocket League Tournament Manager plugin for BakkesMod", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RLTM::onLoad()
{
	_globalCvarManager = cvarManager;

	ix::initNetSystem();
	HookEvents();
	InitSocket();

	cvarManager->log("RLTM Plugin loaded");
}

void RLTM::onUnload()
{
	socket.stop();
	UnhookEvents();
	ix::uninitNetSystem();

	cvarManager->log("RLTM Plugin unloaded");
}

void RLTM::HookEvents()
{
	if (hooked) return;

	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&RLTM::FetchGameData, this, 1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&RLTM::FetchGameData, this, 0));

	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", std::bind(&RLTM::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", std::bind(&RLTM::SetSpectatorUI, this, 0));
	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", std::bind(&RLTM::RemoveStatGraph, this));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", std::bind(&RLTM::SetReady, this));

	hooked = true;
}

void RLTM::UnhookEvents()
{
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.Ball_TA.Explode");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");

	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");
	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");

	hooked = false;
}

void RLTM::InitSocket()
{
	if (socket.getReadyState() != ix::ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3000/?token=1234");
	socket.setHandshakeTimeout(4);
	socket.setPingInterval(45);
	socket.enableAutomaticReconnection();
	socket.setMinWaitBetweenReconnectionRetries(1000);
	socket.setMaxWaitBetweenReconnectionRetries(5000);

	socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
	{
		switch (msg->type)
		{
		case ix::WebSocketMessageType::Open:
			cvarManager->log("Socket connected");
			break;

		case ix::WebSocketMessageType::Message:
			cvarManager->log("Socket message: " + msg->str);
			break;

		case ix::WebSocketMessageType::Error:
			cvarManager->log("Socket error: " + msg->errorInfo.reason);
			break;

		case ix::WebSocketMessageType::Close:
			cvarManager->log("Socket disconnected");
			break;
		}
	});
	socket.start();
}

void RLTM::SendSocketMessage(std::string topic, json message)
{
	if (socket.getReadyState() != ix::ReadyState::Open) return;

	json data;
	data["topic"] = topic;
	data["message"] = message;

	socket.send(data.dump());
}

ServerWrapper RLTM::GetServerWrapper()
{
	ServerWrapper localServer = gameWrapper->GetGameEventAsServer();
	ServerWrapper onlineServer = gameWrapper->GetOnlineGame();

	if (!onlineServer.IsNull()) return onlineServer;
	if (!localServer.IsNull()) return localServer;
	return null;
}

void RLTM::FetchGameData(int isReplayingGoal)
{
	ServerWrapper server = GetServerWrapper();

	json message;

	if (!server.IsNull())
	{
		message["value"] = server.GetSecondsRemaining();
		message["score"] = GetGameScore(server);
		message["isOvertime"] = server.GetbOverTime();
		message["isReplay"] = isReplayingGoal;
	}

	SendSocketMessage("game", message);
}

std::array<int, 2> RLTM::GetGameScore(ServerWrapper server)
{
	PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();

	if (teams.IsNull()) return { 0, 0 };

	if (!primaryPlayer.IsNull())
		for (TeamWrapper team : teams)
			if (primaryPlayer.GetTeamNum2() != team.GetTeamNum2())
				return { teams.Get(0).GetScore(), teams.Get(1).GetScore() };

	return { teams.Get(1).GetScore(), teams.Get(0).GetScore() };
}

void RLTM::SetSpectatorUI(int sleep)
{
	ServerWrapper server = GetServerWrapper()

	if (!server.IsNull())
	{
		PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
		if (primaryPlayer.isNull()) return;

		PriWrapper player = primaryPlayer.GetPRI();
		if (!player.isNull() && player.IsSpectator())
			cvarManager->executeCommand("sleep " + std::to_string(sleep) + "; replay_gui hud 1; replay_gui names 1; replay_gui matchinfo 1; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
	}
}

void RLTM::SetReady()
{
	ServerWrapper server = GetServerWrapper();
	if (server.isNull()) return;

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	PriWrapper player = playerController.GetPRI();
	if (!player.IsNull()) player.ServerReadyUp();
	
}

void RLTM::RemoveStatGraph()
{
	EngineTAWrapper engine = gameWrapper->GetEngine();
	if (engine.IsNull()) return;

	StatGraphSystemWrapper statGraphs = engine.GetStatGraphs();
	if (!statGraphs.IsNull()) statGraphs.SetGraphLevel(6);
}
