#include "pch.h"
#include "RLTM.h"


BAKKESMOD_PLUGIN(RLTM, "Rocket League Tournament Manager", plugin_version, PERMISSION_ALL)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RLTM::onLoad()
{
	_globalCvarManager = cvarManager;

	ix::initNetSystem();
	HookEvents();
	InitSocket();

	ServerWrapper server = GetServerWrapper();
	if (server)
	{
		GetMatchData("onLoad");
	}
	else ResetDatas();

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
	if (isHooked) return;

	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded",   std::bind(&RLTM::GetPlayersData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", std::bind(&RLTM::GetPlayersData, this, std::placeholders::_1));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer",     std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged",                   std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState",             std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit",   std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Team_TA.OnScoreUpdated",                 std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated",  std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated",  std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function Engine.WorldInfo.EventPauseChanged",            std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded",    std::bind(&RLTM::GetMatchData, this, std::placeholders::_1));

	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", std::bind(&RLTM::OnStatTickerMessage, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatEvent",         std::bind(&RLTM::OnStatEvent, this, std::placeholders::_1, std::placeholders::_2));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&RLTM::ResetDatas, this));

	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx",  std::bind(&RLTM::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", std::bind(&RLTM::SetSpectatorUI, this, 0));

	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", std::bind(&RLTM::SetStatGraph, this));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", std::bind(&RLTM::SetReady, this));

	isHooked = true;
}

void RLTM::UnhookEvents()
{
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer");
	gameWrapper->UnhookEvent("Function TAGame.PRI_TA.OnTeamChanged");
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit");
	gameWrapper->UnhookEvent("Function TAGame.Team_TA.OnScoreUpdated");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated");
	gameWrapper->UnhookEvent("Function Engine.WorldInfo.EventPauseChanged");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");

	gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");
	gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatEvent");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");

	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");

	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");

	isHooked = false;
}

void RLTM::InitSocket()
{
	if (socket.getReadyState() != ix::ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3000/?token=1234");
	socket.setHandshakeTimeout(4);
	socket.setPingInterval(2);
	socket.enableAutomaticReconnection();
	socket.setMinWaitBetweenReconnectionRetries(1000);
	socket.setMaxWaitBetweenReconnectionRetries(5000);

	socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
	{
		switch (msg->type)
		{
		case ix::WebSocketMessageType::Open:
			cvarManager->log("Socket connected");
			if (!oldData["match"].is_null()) socket.send(oldData["match"].dump());
			if (!oldData["players"].is_null()) socket.send(oldData["players"].dump());
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

void RLTM::SendSocketMessage(std::string topic, json payload)
{
	json data;
	data["topic"] = topic;
	data["payload"] = payload;

	if (data.dump() == oldData[topic].dump()) return;
	oldData[topic] = data;

	if (socket.getReadyState() != ix::ReadyState::Open) return;
	socket.send(data.dump());
}

ServerWrapper RLTM::GetServerWrapper()
{
	ServerWrapper onlineServer = gameWrapper->GetOnlineGame();
	ServerWrapper localServer = gameWrapper->GetGameEventAsServer();

	if (!onlineServer.IsNull())
	{
		GameSettingPlaylistWrapper playlist = onlineServer.GetPlaylist();
		if (!playlist.IsNull())
		{
			int playlistID = playlist.GetPlaylistId();
			if (playlistID == 6) return onlineServer;
		}
	} 
	if (!localServer.IsNull()) return localServer;
	return NULL;
}

void RLTM::GetPlayersData(std::string caller)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	ArrayWrapper<PriWrapper> players = server.GetPRIs();
	if (!players.IsNull())
	{
		json payload = json::array();
		for (PriWrapper player : players)
		{
			json playerData;
			playerData["name"] = player.GetPlayerName().ToString();
			playerData["uid"] = player.GetUniqueIdWrapper().GetIdString();
			playerData["bot"] = player.GetbBot();

			payload.push_back(playerData);
		}

		SendSocketMessage("players", payload);
	}

	GetMatchData(caller);
}

void RLTM::GetMatchData(std::string caller)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	if (caller == "Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit" && oldData["match"]["payload"]["isStarted"] == 1) return;

	json payload;
	payload["map"] = gameWrapper->GetCurrentMap();
	payload["score"] = GetGameScore(server);
	payload["duration"] = server.GetGameTime();
	payload["statistics"] = GetGameStatistics(server);
	payload["isUnlimited"] = server.GetbUnlimitedTime();
	payload["isStarted"] = server.GetbBallHasBeenHit();
	payload["isPaused"] = gameWrapper->IsPaused() ? 1 : 0;
	payload["isOvertime"] = server.GetbOverTime();
	payload["isEnded"] = server.GetbMatchEnded();
	payload["isReplay"] = server.GetbBallHasBeenHit() && !server.GetbRoundActive() && !server.GetbMatchEnded() && caller != "Function GameEvent_TA.Countdown.BeginState" ? 1 : 0;

	if (server.GetbMatchEnded() && server.GetbOverTime()) payload["time"] = oldData["match"]["payload"]["time"];
	else payload["time"] = server.GetSecondsRemaining();

	SendSocketMessage("match", payload);
}

std::array<int, 2> RLTM::GetGameScore(ServerWrapper server)
{
	PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();

	if (teams.IsNull()) return { 0, 0 };

	for (TeamWrapper team : teams)
		if (!primaryPlayer.IsNull())
			if (primaryPlayer.GetTeamNum2() != team.GetTeamNum2())
				return { teams.Get(0).GetScore(), teams.Get(1).GetScore() };

	return { teams.Get(1).GetScore(), teams.Get(0).GetScore() };
}

json RLTM::GetGameStatistics(ServerWrapper server)
{
	ArrayWrapper<PriWrapper> players = server.GetPRIs();
	if (players.IsNull()) return json::array();

	json statistics = json::array();
	for (PriWrapper player : players)
	{
		if (player.IsNull() || player.GetTeamNum() == 255) continue;

		json playerData;
		playerData["name"] = player.GetPlayerName().ToString();
		playerData["uid"] = player.GetUniqueIdWrapper().GetIdString();
		playerData["score"] = player.GetMatchScore();
		playerData["goals"] = player.GetMatchGoals();
		playerData["ownGoals"] = player.GetMatchOwnGoals();
		playerData["shots"] = player.GetMatchShots();
		playerData["assists"] = player.GetMatchAssists();
		playerData["saves"] = player.GetMatchSaves();
		playerData["ballTouches"] = player.GetBallTouches();
		playerData["carTouches"] = player.GetCarTouches();
		playerData["demolishes"] = player.GetMatchDemolishes();
		playerData["mvp"] = player.GetbMatchMVP();
		playerData["team"] = player.GetTeamNum();

		CarWrapper car = player.GetCar();
		if (car.IsNull()) playerData["car"] = {};
		else
		{
			auto boost = car.GetBoostComponent();
			if (boost.IsNull()) playerData["car"]["boost"] = -1;
			else playerData["car"]["boost"] = boost.GetCurrentBoostAmount();

			playerData["car"]["speed"] = (car.GetVelocity().magnitude() * 0.036f) + 0.5f;
			playerData["car"]["isSuperSonic"] = car.GetbSuperSonic();
			playerData["car"]["isOnWall"] = car.IsOnWall();
			playerData["car"]["isOnGround"] = car.IsOnGround();
			playerData["car"]["isInGoal"] = car.GetbWasInGoalZone();
			playerData["car"]["isDodging"] = car.IsDodging();
			playerData["car"]["asFlip"] = car.HasFlip();
		}

		statistics.push_back(playerData);
	}

	return statistics;
}

void RLTM::OnStatTickerMessage(ServerWrapper _server, void* params)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	StatTickerParams* pStruct = (StatTickerParams*)params;
	PriWrapper player = PriWrapper(pStruct->Receiver);
	StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	GetPlayerStatData(player, event);
}

void RLTM::OnStatEvent(ServerWrapper _server, void* params)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	StatEventParams* pStruct = (StatEventParams*)params;
    PriWrapper player = PriWrapper(pStruct->PRI);
    StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	GetPlayerStatData(player, event);
}

void RLTM::GetPlayerStatData(PriWrapper player, StatEventWrapper event)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	if (player.IsNull() || event.IsNull()) return;

	json payload;
	payload["name"] = player.GetPlayerName().ToString();
	payload["uid"] = player.GetUniqueIdWrapper().GetIdString();
	payload["event_name"] = event.GetEventName();

	SendSocketMessage("statistic", payload);
}

void RLTM::ResetDatas()
{
	SendSocketMessage("match", {});
	SendSocketMessage("players", {});
}

void RLTM::SetSpectatorUI(int sleep)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
	if (primaryPlayer.IsNull()) return;

	PriWrapper player = primaryPlayer.GetPRI();
	if (!player.IsNull() && player.IsSpectator())
		cvarManager->executeCommand("sleep " + std::to_string(sleep) + "; replay_gui hud 1; replay_gui names 1; replay_gui matchinfo 1; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
}

void RLTM::SetReady()
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	PriWrapper player = playerController.GetPRI();
	if (!player.IsNull()) player.ServerReadyUp();
}

void RLTM::SetStatGraph()
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	EngineTAWrapper engine = gameWrapper->GetEngine();
	if (engine.IsNull()) return;

	StatGraphSystemWrapper statGraphs = engine.GetStatGraphs();
	if (!statGraphs.IsNull()) statGraphs.SetGraphLevel(6);
}
