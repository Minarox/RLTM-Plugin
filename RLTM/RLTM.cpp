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
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Team_TA.OnScoreUpdated", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function Engine.WorldInfo.EventPauseChanged", bind(&RLTM::GetMatchData, this, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", bind(&RLTM::GetMatchData, this, placeholders::_1));

	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", bind(&RLTM::SetReplayState, this, true, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", bind(&RLTM::SetReplayState, this, false, placeholders::_1));

	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&RLTM::GetPlayerStatData, this, placeholders::_1, placeholders::_2));

	gameWrapper->HookEventPost("Function Engine.GameViewportClient.Tick", bind(&RLTM::GetEntitiesData, this));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&RLTM::ResetDatas, this));

	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", bind(&RLTM::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", bind(&RLTM::SetSpectatorUI, this, 0));

	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", bind(&RLTM::SetStatGraph, this));

	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", bind(&RLTM::SetReady, this));
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
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated");
	gameWrapper->UnhookEvent("Function Engine.WorldInfo.EventPauseChanged");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");

	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");

	gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");

	gameWrapper->UnhookEventPost("Function Engine.GameViewportClient.Tick");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");

	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");

	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");

	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");
}


/*
	--- WebSocket ---
*/

void RLTM::InitSocket()
{
	if (socket.getReadyState() != ix::ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3300?token=");
	socket.setHandshakeTimeout(3);
	socket.setPingInterval(1);
	socket.enableAutomaticReconnection();
	socket.setMinWaitBetweenReconnectionRetries(500);
	socket.setMaxWaitBetweenReconnectionRetries(500);
	socket.enablePerMessageDeflate();
	socket.addSubProtocol("json");

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

	if (data["payload"] == oldData[topic]) return;
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

void RLTM::SetReplayState(bool state, string caller)
{
	if (isReplay == state) return;
	isReplay = state;
	GetMatchData(caller);
}

void RLTM::GetMatchData(string caller)
{
	if (caller == "onLoad") ResetDatas();

	ServerWrapper server = GetServerWrapper();
	if (!server) return;
	if (caller == "Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit" && oldData[eventToTopic[MATCH]]["isStarted"] == true) return;

	json payload = json::object();
	payload["arenaCode"] = gameWrapper->GetCurrentMap();
	payload["score"] = GetScore(server);
	payload["duration"] = server.GetGameTime();
	payload["isUnlimited"] = (bool) server.GetbUnlimitedTime();
	payload["isStarted"] = oldData[eventToTopic[MATCH]]["isStarted"] == true || server.GetbBallHasBeenHit() || caller == "Function GameEvent_TA.Countdown.BeginState";
	payload["isPaused"] = gameWrapper->IsPaused() && !server.GetbMatchEnded();
	payload["isOvertime"] = (bool) server.GetbOverTime();
	payload["isEnded"] = (bool) server.GetbMatchEnded();
	payload["isReplay"] = isReplay;
	payload["statistics"] = GetStatistics(server);

	if (server.GetbMatchEnded() && server.GetbOverTime()) payload["clock"] = oldData[eventToTopic[MATCH]]["clock"];
	else payload["clock"] = server.GetSecondsRemaining();

	SendSocketMessage(MATCH, payload);

	if (payload["isStarted"] == true && payload["isPaused"] == false && payload["isEnded"] == false)
	{
		SetReplayAutoSave(true);

		if (!threadRunning)
		{
			threadRunning = true;
			thread asyncThread(&RLTM::SendEntitiesData, this);
			asyncThread.detach();
		}
	}
	else threadRunning = false;
}

json RLTM::GetScore(ServerWrapper server)
{
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	json score = json::array();

	if (!teams.IsNull())
	{
		if (teams.Count() > 1)
		{
			TeamWrapper team0 = teams.Get(0);
			TeamWrapper team1 = teams.Get(1);

			if (!team0.IsNull() && !team1.IsNull())
			{
				score += team0.GetScore();
				score += team1.GetScore();

				return score;
			}
		}
	}

	return score;
}

json RLTM::GetStatistics(ServerWrapper server)
{
	json statistics = json::object();
	ArrayWrapper<PriWrapper> players = server.GetPRIs();

	if (!players.IsNull())
	{
		for (PriWrapper player : players)
		{
			if (player.IsNull()) continue;
			if (player.GetTeamNum() == 255) continue;

			string playerName = player.GetPlayerName().ToString();
			string playerUID = player.GetUniqueIdWrapper().GetIdString();

			json playerData = json::object();
			playerData["uid"] = playerUID;
			playerData["name"] = playerName;
			playerData["bot"] = (bool) player.GetbBot();
			playerData["teamIndex"] = player.GetTeamNum();
			playerData["mvp"] = (bool) player.GetbMatchMVP();
			playerData["score"] = player.GetMatchScore();
			playerData["goals"] = player.GetMatchGoals();
			playerData["shots"] = player.GetMatchShots();
			playerData["assists"] = player.GetMatchAssists();
			playerData["saves"] = player.GetMatchSaves();
			playerData["ballTouches"] = player.GetBallTouches();
			playerData["carTouches"] = player.GetCarTouches();

			json data = oldData[eventToTopic[MATCH]]["statistics"][playerUID + '|' + playerName];

			for (string event : { "Demolish", "Demolition", "AerialGoal", "BackwardsGoal", "BicycleGoal", "LongGoal", "TurtleGoal", "PoolShot", "OvertimeGoal", "HatTrick", "Playmaker", "EpicSave", "Savior", "Center", "Clear", "FirstTouch", "BreakoutDamage", "BreakoutDamageLarge", "LowFive", "HighFive", "HoopsSwishGoal", "BicycleHit", "OwnGoal", "KO_Winner", "KO_Knockout", "KO_DoubleKO", "KO_TripleKO", "KO_Death", "KO_LightHit", "KO_HeavyHit", "KO_AerialLightHit", "KO_AerialHeavyHit", "KO_HitTaken", "KO_BlockTaken", "KO_Grabbed", "KO_Thrown", "KO_LightBlock", "KO_HeavyBlock", "KO_PlayerGrabbed", "KO_PlayerThrown" })
				playerData[event] = data[event].is_null() ? 0 : (int) data[event];

			statistics[playerUID + '|' + playerName] = playerData;
		}
	}

	return statistics;
}

void RLTM::GetPlayerStatData(ServerWrapper _server, void* params)
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	StatTickerParams* pStruct = (StatTickerParams*)params;
	PriWrapper player = PriWrapper(pStruct->Receiver);
	StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	if (player.IsNull() || event.IsNull()) return;

	string playerName = player.GetPlayerName().ToString();
	string playerUID = player.GetUniqueIdWrapper().GetIdString();
	string tick = event.GetEventName() + '|' + playerUID + '|' + playerName;

	if (tickBuffer == tick)
	{
		tickBuffer = "";
		return;
	}
	else tickBuffer = tick;

	json data = oldData[eventToTopic[MATCH]]["statistics"][playerUID + '|' + playerName];
	if (!data[event.GetEventName()].is_null())
		oldData[eventToTopic[MATCH]]["statistics"][playerUID + '|' + playerName][event.GetEventName()] = data[event.GetEventName()] + 1;

	json payload = json::object();
	payload["uid"] = playerUID;
	payload["name"] = playerName;
	payload["bot"] = (bool) player.GetbBot();
	payload["eventName"] = event.GetEventName();

	SendSocketMessage(STATISTIC, payload);
}

void RLTM::GetEntitiesData()
{
	ServerWrapper server = GetServerWrapper();
	if (!server) return;

	GetPlayersData(server);

	if (oldData[eventToTopic[MATCH]]["isStarted"] == false || oldData[eventToTopic[MATCH]]["isEnded"] == true || oldData[eventToTopic[MATCH]]["isPaused"] == true) return;

	json payload = json::object();
	payload["balls"] = json::array();

	auto balls = server.GetGameBalls();
	if (!balls.IsNull())
	{
		int i = 0;
		for (int index = 0; index < balls.Count(); index++)
		{
			BallWrapper ball = balls.Get(i);
			if (ball.IsNull()) continue;

			Vector location = ball.GetLocation();
			// Vector velocity = ball.GetVelocity();
			// Rotator rotation = ball.GetRotation();

			json ballData = json::object();
			ballData["radius"] = (int) ball.GetRadius();
			ballData["location"] = { (int) location.X, (int) location.Y, (int) location.Z };
			//ballData["velocity"] = { (int) velocity.X, (int) velocity.Y, (int) velocity.Z };
			//ballData["rotation"] = { rotation.Pitch, rotation.Yaw, rotation.Roll };

			payload["balls"] += ballData;
			i++;
		}
	}

	payload["cars"] = json::array();
	ArrayWrapper<PriWrapper> players = server.GetPRIs();

	if (!players.IsNull())
	{
		for (PriWrapper player : players)
		{
			if (player.IsNull()) continue;
			if(player.GetTeamNum() == 255) continue;

			CarWrapper car = player.GetCar();
			if (car.IsNull()) continue;

			Vector location = car.GetLocation();
			// Vector velocity = car.GetVelocity();
			// Rotator rotation = car.GetRotation();

			json carData = json::object();
			carData["uid"] = player.GetUniqueIdWrapper().GetIdString();
			carData["name"] = player.GetPlayerName().ToString();
			carData["bot"] = (bool) player.GetbBot();
			carData["teamIndex"] = player.GetTeamNum();
			carData["speed"] = (int) ((car.GetVelocity().magnitude() * 0.036f) + 0.5f);
			carData["location"] = { (int) location.X, (int) location.Y, (int) location.Z };
			//carData["velocity"] = { (int) velocity.X, (int) velocity.Y, (int) velocity.Z };
			//carData["rotation"] = { rotation.Pitch, rotation.Yaw, rotation.Roll };
			carData["isSuperSonic"] = (bool) car.GetbSuperSonic();
			carData["isOnWall"] = car.IsOnWall();
			carData["isOnGround"] = car.IsOnGround();
			carData["isInGoal"] = (bool) car.GetbWasInGoalZone();
			carData["isDodging"] = car.IsDodging();
			carData["asFlip"] = (bool) car.HasFlip();

			auto boost = car.GetBoostComponent();
			if (boost.IsNull()) carData["boost"] = 0;
			else carData["boost"] = (int) (boost.GetCurrentBoostAmount() * 100);

			payload["cars"] += carData;
		}
	}

	entitiesData = payload;
}

void RLTM::SendEntitiesData()
{
	while (threadRunning)
	{
		SendSocketMessage(ENTITIES, entitiesData);
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

void RLTM::GetPlayersData(ServerWrapper server)
{
	json playersArray = json::array();
	ArrayWrapper<PriWrapper> players = server.GetPRIs();

	if (players.IsNull() || !players.Count()) return;

	for (PriWrapper player : players)
	{
		if (player.IsNull()) continue;

		json playerData = json::object();
		playerData["uid"] = player.GetUniqueIdWrapper().GetIdString();
		playerData["name"] = player.GetPlayerName().ToString();
		playerData["bot"] = (bool) player.GetbBot();
		playerData["teamIndex"] = player.GetTeamNum();

		CarWrapper car = player.GetCar();
		if (car.IsNull()) playerData["carId"] = 0;
		else playerData["carId"] = car.GetLoadoutBody();

		playersArray += playerData;
	}

	SendSocketMessage(PLAYERS, playersArray);
}

void RLTM::ResetDatas()
{
	SetReplayAutoSave(false);
	threadRunning = false;
	isReplay = false;

	for (Event event : { MATCH, ENTITIES, PLAYERS })
		SendSocketMessage(event, {});
}


/*
	--- Game Replays ---
*/
void RLTM::SetReplayAutoSave(bool status)
{
	if (autoSaveReplay == status) return;
	autoSaveReplay = status;
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
	{
		cvarManager->executeCommand("sleep " + to_string(sleep) + "; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
	}
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
