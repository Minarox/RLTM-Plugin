#include "pch.h"
#include "RLTM.h"


BAKKESMOD_PLUGIN(RLTM, "Rocket League Tournament Manager", plugin_version, PLUGINTYPE_SPECTATOR)

shared_ptr<CVarManagerWrapper> _globalCvarManager;


/*
	--- Boilerplate ---
*/

void RLTM::onLoad()
{
	_globalCvarManager = cvarManager;

	ix::initNetSystem();
	HookEvents();
	GetMatchData("onLoad");
	InitSocket();

	cvarManager->log("RLTM Plugin loaded");
}

void RLTM::onUnload()
{
	UnhookEvents();
	ResetDatas();
	socket.stop();
	ix::uninitNetSystem();

	cvarManager->log("RLTM Plugin unloaded");
}


/*
	--- Hooks ---
*/

void RLTM::HookEvents()
{
	if (isHooked) return;

	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Team_TA.OnScoreUpdated", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function Engine.WorldInfo.EventPauseChanged", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", bind(&RLTM::GetMatchData, this, placeholders::_1));

	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&RLTM::OnStatTickerMessage, this, placeholders::_1, placeholders::_2));
	//gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatEvent", bind(&RLTM::OnStatEvent, this, placeholders::_1, placeholders::_2));

	//gameWrapper->HookEventPost("Function Engine.GameViewportClient.Tick", bind(&RLTM::GetEntitiesData, this));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&RLTM::ResetDatas, this));

	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", bind(&RLTM::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", bind(&RLTM::SetSpectatorUI, this, 0));

	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", bind(&RLTM::SetStatGraph, this));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", bind(&RLTM::SetReady, this));

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

	gameWrapper->UnhookEventPost("Function Engine.GameViewportClient.Tick");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");

	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");

	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");

	isHooked = false;
}


/*
	--- WebSocket ---
*/

void RLTM::InitSocket()
{
	if (socket.getReadyState() != ix::ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3000/game?token=457218CD39CB96BF59086E83C4D21AC6");
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
				for (auto& [key, value] : oldData.items())
				{
					json data = json::object();
					data["topic"] = key;
					data["payload"] = value;

					socket.send(data.dump());
				}
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

void RLTM::SendSocketMessage(Event event, json payload)
{
	string topic = eventToTopic[event];

	json data = json::object();
	data["topic"] = topic;
	data["payload"] = payload;

	if (data["payload"].dump() == oldData[topic].dump()) return;
	if (topic != eventToTopic[STATISTIC]) oldData[topic] = data["payload"];

	if (socket.getReadyState() != ix::ReadyState::Open) return;
	socket.send(data.dump());
}


/*
	--- Game Data ---
*/

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

void RLTM::GetMatchData(string caller)
{
	ServerWrapper server = GetServerWrapper();
	if (!server && caller == "onLoad") ResetDatas();
	if (!server) return;

	if (caller == "Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit" && oldData[eventToTopic[MATCH]]["isStarted"] == 1) return;

	json payload = json::object();
	payload["id"] = server.GetMatchGUID();
	payload["map"] = gameWrapper->GetCurrentMap();
	payload["score"] = GetScore(server);
	payload["duration"] = server.GetGameTime();
	payload["isUnlimited"] = (bool) server.GetbUnlimitedTime();
	payload["isStarted"] = oldData[eventToTopic[MATCH]]["isStarted"] == 1 || server.GetbBallHasBeenHit() || caller == "Function GameEvent_TA.Countdown.BeginState";
	payload["isPaused"] = gameWrapper->IsPaused() && !server.GetbMatchEnded();
	payload["isOvertime"] = (bool) server.GetbOverTime();
	payload["isEnded"] = (bool) server.GetbMatchEnded();
	payload["isReplay"] = payload["isStarted"] == 1 && !server.GetbRoundActive() && !server.GetbMatchEnded() && caller != "Function GameEvent_TA.Countdown.BeginState" && (!server.GetbOverTime() || server.GetbOverTime() && server.GetSecondsRemaining() != 0);

	if (server.GetbMatchEnded() && server.GetbOverTime()) payload["time"] = oldData[eventToTopic[MATCH]]["time"];
	else payload["time"] = server.GetSecondsRemaining();

	SendSocketMessage(MATCH, payload);

	GetStatisticsData(server);

	if (payload["isEnded"] == 0 && payload["isStarted"] == 1) SetReplayAutoSave(true);
}

array<int, 2> RLTM::GetScore(ServerWrapper server)
{
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();

	if (!teams.IsNull())
		if (teams.Count() > 1)
		{
			TeamWrapper team0 = teams.Get(0);
			TeamWrapper team1 = teams.Get(1);

			if (!team0.IsNull() && !team1.IsNull())
				return { team0.GetScore(), team1.GetScore() };
		}

	return json::array();
}

void RLTM::GetStatisticsData(ServerWrapper server)
{
	ArrayWrapper<PriWrapper> players = server.GetPRIs();
	if (players.IsNull() || !players.Count()) return;

	json payload = json::array();
	for (PriWrapper player : players)
	{
		if (player.IsNull() || player.GetTeamNum() == 255) continue;

		json playerData = json::object();
		playerData["mvp"] = (bool) player.GetbMatchMVP();
		playerData["score"] = player.GetMatchScore();
		playerData["goals"] = player.GetMatchGoals();
		playerData["shots"] = player.GetMatchShots();
		playerData["assists"] = player.GetMatchAssists();
		playerData["saves"] = player.GetMatchSaves();
		playerData["ballTouches"] = player.GetBallTouches();
		playerData["carTouches"] = player.GetCarTouches();

		string playerName = player.GetPlayerName().ToString();
		string playerUID = player.GetUniqueIdWrapper().GetIdString();
		json data = oldData[eventToTopic[STATISTICS]][player.GetTeamNum()][playerUID + '_' + playerName];

		for (string event : { "Demolish", "Demolition", "AerialGoal", "BackwardsGoal", "BicycleGoal", "LongGoal", "TurtleGoal", "PoolShot", "OvertimeGoal", "HatTrick", "Playmaker", "EpicSave", "Savior", "Center", "Clear", "FirstTouch", "BreakoutDamage", "BreakoutDamageLarge", "LowFive", "HighFive", "HoopsSwishGoal", "BicycleHit", "OwnGoal", "KO_Winner", "KO_Knockout", "KO_DoubleKO", "KO_TripleKO", "KO_Death", "KO_LightHit", "KO_HeavyHit", "KO_AerialLightHit", "KO_AerialHeavyHit", "KO_HitTaken", "KO_BlockTaken", "KO_Grabbed", "KO_Thrown", "KO_LightBlock", "KO_HeavyBlock", "KO_PlayerGrabbed", "KO_PlayerThrown" })
			playerData[event] = !data[event].is_null() ? (int) data[event] : 0;

		payload[player.GetTeamNum()][playerUID + '_' + playerName] = playerData;
	}

	SendSocketMessage(STATISTICS, payload);
}

void RLTM::OnStatTickerMessage(ServerWrapper _server, void* params)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	StatTickerParams* pStruct = (StatTickerParams*)params;
	PriWrapper player = PriWrapper(pStruct->Receiver);
	StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	if (tickBuffer == event.GetEventName())
	{
		tickBuffer = "";
		return;
	}
	else tickBuffer = event.GetEventName();

	GetPlayerStatData(player, event, server);
}

void RLTM::OnStatEvent(ServerWrapper _server, void* params)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	StatEventParams* pStruct = (StatEventParams*)params;
	PriWrapper player = PriWrapper(pStruct->PRI);
	StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	GetPlayerStatData(player, event, server);
}

void RLTM::GetPlayerStatData(PriWrapper player, StatEventWrapper event, ServerWrapper server)
{
	if (player.IsNull() || event.IsNull()) return;

	string playerName = player.GetPlayerName().ToString();
	string playerUID = player.GetUniqueIdWrapper().GetIdString();
	json data = oldData[eventToTopic[STATISTICS]][player.GetTeamNum()][playerUID + '_' + playerName];

	if (!data[event.GetEventName()].is_null())
		oldData[eventToTopic[STATISTICS]][player.GetTeamNum()][playerUID + '_' + playerName][event.GetEventName()] = data[event.GetEventName()] + 1;

	json payload = json::object();
	payload["player"] = playerUID + '_' + playerName;
	payload["event"] = event.GetEventName();

	SendSocketMessage(STATISTIC, payload);

	GetStatisticsData(server);
}

void RLTM::GetEntitiesData()
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;
	if (oldData[eventToTopic[MATCH]]["isStarted"] == 0 || server.GetbMatchEnded() || oldData[eventToTopic[MATCH]]["IsPaused"] == 1) return;

	json payload = json::object();
	payload["balls"] = json::array();
	payload["cars"] = json::array();

	auto balls = server.GetGameBalls();
	if (!balls.IsNull() && balls.Count())
	{
		int i = 0;
		for (int index = 0; index < balls.Count(); index++)
		{
			BallWrapper ball = balls.Get(i);
			if (ball.IsNull()) continue;

			Vector location = ball.GetLocation();
			Vector velocity = ball.GetVelocity();
			Rotator rotation = ball.GetRotation();

			json ballData = json::object();
			ballData["radius"] = (int) ball.GetRadius();
			ballData["visualRadius"] = (int) ball.GetVisualRadius();
			ballData["location"] = { (int) location.X, (int) location.Y, (int) location.Z };
			ballData["velocity"] = { (int) velocity.X, (int) velocity.Y, (int) velocity.Z };
			ballData["rotation"] = { rotation.Pitch, rotation.Yaw, rotation.Roll };
			
			payload["balls"][i] = ballData;
			i++;
		}
	}

	ArrayWrapper<PriWrapper> players = server.GetPRIs();
	if (!players.IsNull() && players.Count())
	{
		for (PriWrapper player : players)
		{
			if (player.IsNull() || player.GetTeamNum() == 255) continue;

			CarWrapper car = player.GetCar();
			if (car.IsNull()) continue;

			string playerName = player.GetPlayerName().ToString();
			string playerUID = player.GetUniqueIdWrapper().GetIdString();

			Vector location = car.GetLocation();
			Vector velocity = car.GetVelocity();
			Rotator rotation = car.GetRotation();

			json carData = json::object();
			carData["speed"] = (int) ((car.GetVelocity().magnitude() * 0.036f) + 0.5f);
			carData["location"] = { (int) location.X, (int) location.Y, (int) location.Z };
			carData["velocity"] = { (int) velocity.X, (int) velocity.Y, (int) velocity.Z };
			carData["rotation"] = { rotation.Pitch, rotation.Yaw, rotation.Roll };
			carData["isSuperSonic"] = (bool) car.GetbSuperSonic();
			carData["isOnWall"] = car.IsOnWall();
			carData["isOnGround"] = car.IsOnGround();
			carData["isInGoal"] = (bool) car.GetbWasInGoalZone();
			carData["isDodging"] = car.IsDodging();
			carData["asFlip"] = (bool) car.HasFlip();

			auto boost = car.GetBoostComponent();
			if (boost.IsNull()) carData["boost"] = -1;
			else carData["boost"] = (int) (boost.GetCurrentBoostAmount() * 100);

			payload["cars"][player.GetTeamNum()][playerUID + '_' + playerName] = carData;
		}
	}

	SendSocketMessage(ENTITIES, payload);
}

void RLTM::ResetDatas()
{
	SetReplayAutoSave(false);

	for (Event event : { MATCH, STATISTICS, ENTITIES })
		SendSocketMessage(event, {});
}


/*
	--- Game Replays ---
*/
void RLTM::SetReplayAutoSave(bool status)
{
	cvarManager->executeCommand("ranked_autosavereplay_all " + to_string(status ? 1 : 0), false);
}


/*
	--- Game HUD ---
*/

void RLTM::SetSpectatorUI(int sleep)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
	if (primaryPlayer.IsNull()) return;

	PriWrapper player = primaryPlayer.GetPRI();
	if (!player.IsNull() && player.IsSpectator())
		cvarManager->executeCommand("sleep " + to_string(sleep) + "; replay_gui hud 1; replay_gui names 1; replay_gui matchinfo 1; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
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

void RLTM::SetReady()
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	PriWrapper player = playerController.GetPRI();
	if (!player.IsNull()) player.ServerReadyUp();
}
