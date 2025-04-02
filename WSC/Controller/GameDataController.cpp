#include "GameDataController.h"

ServerWrapper GameDataController::GetServerWrapper(GameWrapper* gameWrapper)
{
	ServerWrapper onlineServer = gameWrapper->GetOnlineGame();
	ServerWrapper localServer = gameWrapper->GetGameEventAsServer();

	if (onlineServer)
	{
		GameSettingPlaylistWrapper playlist = onlineServer.GetPlaylist();
		if (playlist)
		{
			int playlistID = playlist.GetPlaylistId();
			if (playlistID == 6) return onlineServer;
		}
	}
	if (localServer) return localServer;

	return NULL;
}

void GameDataController::GetMatchData(GameWrapper* gameWrapper, string caller, json& oldData, bool& isReplay, bool& threadRunning, json& entitiesData)
{
	if (caller == "onLoad") ResetDatas(oldData, threadRunning, isReplay);

	ServerWrapper server = GetServerWrapper(gameWrapper);
	if (!server) return;

	if (caller == "Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit" && oldData[eventToTopic[MATCH]]["isStarted"] == true) return;
	if (caller == "Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer") SetSpectator(gameWrapper);

	GetPlayersData(server, oldData);

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
	payload["statistics"] = payload["isStarted"] ? GetStatistics(server, oldData) : json::object();

	if (server.GetbMatchEnded() && server.GetbOverTime()) payload["clock"] = oldData[eventToTopic[MATCH]]["clock"];
	else payload["clock"] = server.GetSecondsRemaining();

	SendSocketMessage(MATCH, payload, oldData);

	if (payload["isStarted"] == true && payload["isPaused"] == false && payload["isEnded"] == false)
	{
		SetReplayAutoSave(true);

		if (!threadRunning)
		{
			threadRunning = true;
			thread asyncThread(&GameDataController::SendEntitiesData, this, ref(entitiesData), ref(threadRunning));
			asyncThread.detach();
		}
	}
	else threadRunning = false;
}

json GameDataController::GetScore(ServerWrapper server)
{
	json score = json::array();
	if (!server) return score;

	ArrayWrapper<TeamWrapper> teams = server.GetTeams();

	if (teams.Count() > 1)
	{
		TeamWrapper team0 = teams.Get(0);
		TeamWrapper team1 = teams.Get(1);

		if (team0 && team1)
		{
			score += team0.GetScore();
			score += team1.GetScore();

			return score;
		}
	}

	return score;
}

json GameDataController::GetStatistics(ServerWrapper server, json& oldData)
{
	json statistics = json::object();
	if (!server) return statistics;

	ArrayWrapper<PriWrapper> players = server.GetPRIs();

	for (PriWrapper player : players)
	{
		if (!player) continue;
		if (player.GetTeamNum() == 255) continue;

		UniqueIDWrapper playerUID = player.GetUniqueIdWrapper();
		UnrealStringWrapper playerName = player.GetPlayerName();
		if (!playerName) continue;

		json playerData = json::object();
		playerData["uid"] = playerUID.GetIdString();
		playerData["name"] = playerName.ToString();
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

		json data = oldData[eventToTopic[MATCH]]["statistics"][playerUID.GetIdString() + '|' + playerName.ToString()];

		for (string event : { "Demolish", "Demolition", "AerialGoal", "BackwardsGoal", "BicycleGoal", "LongGoal", "TurtleGoal", "PoolShot", "OvertimeGoal", "HatTrick", "Playmaker", "EpicSave", "Savior", "Center", "Clear", "FirstTouch", "BreakoutDamage", "BreakoutDamageLarge", "LowFive", "HighFive", "HoopsSwishGoal", "BicycleHit", "OwnGoal", "KO_Winner", "KO_Knockout", "KO_DoubleKO", "KO_TripleKO", "KO_Death", "KO_LightHit", "KO_HeavyHit", "KO_AerialLightHit", "KO_AerialHeavyHit", "KO_HitTaken", "KO_BlockTaken", "KO_Grabbed", "KO_Thrown", "KO_LightBlock", "KO_HeavyBlock", "KO_PlayerGrabbed", "KO_PlayerThrown" })
			playerData[event] = data[event].is_null() ? 0 : (int) data[event];

		statistics[playerUID.GetIdString() + '|' + playerName.ToString()] = playerData;
	}

	return statistics;
}

void GameDataController::GetPlayerStatData(ServerWrapper server, void* params, json& oldData, string& tickBuffer)
{
	if (!server) return;

	StatTickerParams* pStruct = (StatTickerParams*)params;
	PriWrapper player = PriWrapper(pStruct->Receiver);
	StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	if (!player || !event) return;

	UniqueIDWrapper playerUID = player.GetUniqueIdWrapper();
	UnrealStringWrapper playerName = player.GetPlayerName();
	if (!playerName) return;

	string tick = event.GetEventName() + '|' + playerUID.GetIdString() + '|' + playerName.ToString();

	if (tickBuffer == tick)
	{
		tickBuffer = "";
		return;
	}
	else tickBuffer = tick;

	json data = oldData[eventToTopic[MATCH]]["statistics"][playerUID.GetIdString() + '|' + playerName.ToString()];
	if (!data[event.GetEventName()].is_null())
		oldData[eventToTopic[MATCH]]["statistics"][playerUID.GetIdString() + '|' + playerName.ToString()][event.GetEventName()] = data[event.GetEventName()] + 1;

	json payload = json::object();
	payload["uid"] = playerUID.GetIdString();
	payload["name"] = playerName.ToString();
	payload["bot"] = (bool) player.GetbBot();
	payload["eventName"] = event.GetEventName();

	SendSocketMessage(STATISTIC, payload, oldData);
}

void GameDataController::GetEntitiesData(ServerWrapper server, json& oldData, json& entitiesData)
{
	if (!server) return;
	if (!(oldData[eventToTopic[MATCH]]["isStarted"] == true && oldData[eventToTopic[MATCH]]["isEnded"] == false && oldData[eventToTopic[MATCH]]["isPaused"] == false)) return;

	GetPlayersData(server, oldData);

	json payload = json::object();
	payload["balls"] = json::array();

	ArrayWrapper<BallWrapper> balls = server.GetGameBalls();

	for (BallWrapper ball : balls)
	{
		if (!ball) continue;

		Vector location = ball.GetLocation();

		json ballData = json::object();
		ballData["speed"] = (int) ((ball.GetVelocity().magnitude() * 0.036f) + 0.5f);
		ballData["location"] = { (int)location.X, (int)location.Y, (int)location.Z };

		payload["balls"] += ballData;
	}

	payload["cars"] = json::array();
	ArrayWrapper<PriWrapper> players = server.GetPRIs();

	for (PriWrapper player : players)
	{
		if (!player) continue;
		if (player.GetTeamNum() == 255) continue;

		CarWrapper car = player.GetCar();
		if (!car) continue;

		UniqueIDWrapper playerUID = player.GetUniqueIdWrapper();
		UnrealStringWrapper playerName = player.GetPlayerName();
		if (!playerName) continue;

		Vector location = car.GetLocation();

		json carData = json::object();
		carData["uid"] = playerUID.GetIdString();
		carData["name"] = playerName.ToString();
		carData["bot"] = (bool) player.GetbBot();
		carData["teamIndex"] = player.GetTeamNum();
		carData["speed"] = (int) ((car.GetVelocity().magnitude() * 0.036f) + 0.5f);
		carData["location"] = { (int) location.X, (int) location.Y, (int) location.Z };
		carData["isSuperSonic"] = (bool) car.GetbSuperSonic();
		carData["isOnWall"] = car.IsOnWall();
		carData["isOnGround"] = car.IsOnGround();
		carData["isInGoal"] = (bool) car.GetbWasInGoalZone();
		carData["isDodging"] = car.IsDodging();
		carData["asFlip"] = (bool) car.HasFlip();

		BoostWrapper boost = car.GetBoostComponent();
		if (!boost) carData["boost"] = 0;
		else carData["boost"] = (int) (boost.GetCurrentBoostAmount() * 100);

		payload["cars"] += carData;
	}

	entitiesData = payload;
}

void GameDataController::SendEntitiesData(json& entitiesData, bool& threadRunning)
{
	while (threadRunning)
	{
		SendSocketMessage(ENTITIES, entitiesData, oldData);
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

void GameDataController::GetPlayersData(ServerWrapper server, json& oldData)
{
	if (!server) return;

	json playersArray = json::array();
	ArrayWrapper<PriWrapper> players = server.GetPRIs();

	for (PriWrapper player : players)
	{
		if (!player) continue;

		UniqueIDWrapper playerUID = player.GetUniqueIdWrapper();
		UnrealStringWrapper playerName = player.GetPlayerName();
		if (!playerName) return;

		json playerData = json::object();
		playerData["uid"] = playerUID.GetIdString();
		playerData["name"] = playerName.ToString();
		playerData["bot"] = (bool) player.GetbBot();
		playerData["teamIndex"] = player.GetTeamNum();

		CarWrapper car = player.GetCar();
		if (!car) playerData["carId"] = 0;
		else playerData["carId"] = car.GetLoadoutBody();

		playersArray += playerData;
	}

	SendSocketMessage(PLAYERS, playersArray, oldData);
}

void GameDataController::ResetDatas(json& oldData, bool& threadRunning, bool& isReplay)
{
	SetReplayAutoSave(false);
	threadRunning = false;
	isReplay = false;

	for (Event event : { MATCH, ENTITIES, PLAYERS }) SendSocketMessage(event, {}, oldData);
}