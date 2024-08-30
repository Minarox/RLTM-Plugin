#include "pch.h"
#include "RLTM.h"

void RLTM::SetReplayState(bool state, string caller)
{
	if (isReplay == state) return;
	isReplay = state;
	RLTM::GetMatchData(caller);
}

void RLTM::GetMatchData(string caller)
{
	ServerWrapper server = RLTM::GetServerWrapper();
	if (!server && caller == "onLoad") RLTM::ResetDatas();
	if (!server) return;

	if (caller == "Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit" && oldData[eventToTopic[MATCH]]["isStarted"] == true) return;

	json payload = json::object();
	payload["id"] = server.GetMatchGUID();
	payload["map"] = gameWrapper->GetCurrentMap();
	payload["score"] = RLTM::GetScore(server);
	payload["duration"] = server.GetGameTime();
	payload["isUnlimited"] = (bool) server.GetbUnlimitedTime();
	payload["isStarted"] = oldData[eventToTopic[MATCH]]["isStarted"] == true || server.GetbBallHasBeenHit() || caller == "Function GameEvent_TA.Countdown.BeginState";
	payload["isPaused"] = gameWrapper->IsPaused() && !server.GetbMatchEnded();
	payload["isOvertime"] = (bool) server.GetbOverTime();
	payload["isEnded"] = (bool) server.GetbMatchEnded();
	payload["isReplay"] = isReplay;

	if (server.GetbMatchEnded() && server.GetbOverTime()) payload["time"] = oldData[eventToTopic[MATCH]]["time"];
	else payload["time"] = server.GetSecondsRemaining();

	RLTM::SendSocketMessage(MATCH, payload);

	RLTM::GetStatisticsData(server);

	if (payload["isStarted"] == true && payload["isPaused"] == false && payload["isEnded"] == false)
	{
		RLTM::SetReplayAutoSave(true);

		if (!threadRunning)
		{
			threadRunning = true;
			thread asyncThread(&RLTM::SendEntitiesData, this);
			asyncThread.detach();
		}
	}
	else threadRunning = false;
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

	RLTM::SendSocketMessage(STATISTICS, payload);
}

void RLTM::GetPlayerStatData(ServerWrapper _server, void* params)
{
	ServerWrapper server = RLTM::GetServerWrapper();
	if (!server) return;

	StatTickerParams* pStruct = (StatTickerParams*)params;
	PriWrapper player = PriWrapper(pStruct->Receiver);
	StatEventWrapper event = StatEventWrapper(pStruct->StatEvent);

	if (player.IsNull() || event.IsNull()) return;

	string playerName = player.GetPlayerName().ToString();
	string playerUID = player.GetUniqueIdWrapper().GetIdString();
	string tick = event.GetEventName() + '_' + playerUID + '_' + playerName;

	if (tickBuffer == tick)
	{
		tickBuffer = "";
		return;
	}
	else tickBuffer = tick;

	json data = oldData[eventToTopic[STATISTICS]][player.GetTeamNum()][playerUID + '_' + playerName];
	if (!data[event.GetEventName()].is_null())
		oldData[eventToTopic[STATISTICS]][player.GetTeamNum()][playerUID + '_' + playerName][event.GetEventName()] = data[event.GetEventName()] + 1;

	json payload = json::object();
	payload["player"] = playerUID + '_' + playerName;
	payload["event"] = event.GetEventName();

	RLTM::SendSocketMessage(STATISTIC, payload);

	RLTM::GetStatisticsData(server);
}

void RLTM::GetEntitiesData()
{
	ServerWrapper server = RLTM::GetServerWrapper();
	if (!server) return;
	if (oldData[eventToTopic[MATCH]]["isStarted"] == false || server.GetbMatchEnded() || oldData[eventToTopic[MATCH]]["isPaused"] == true) return;

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
			//ballData["radius"] = (int) ball.GetRadius();
			//ballData["visualRadius"] = (int) ball.GetVisualRadius();
			ballData["location"] = { (int) location.X, (int) location.Y, (int) location.Z };
			//ballData["velocity"] = { (int) velocity.X, (int) velocity.Y, (int) velocity.Z };
			//ballData["rotation"] = { rotation.Pitch, rotation.Yaw, rotation.Roll };

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
			//carData["velocity"] = { (int) velocity.X, (int) velocity.Y, (int) velocity.Z };
			//carData["rotation"] = { rotation.Pitch, rotation.Yaw, rotation.Roll };
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

	entitiesData = payload;
}

void RLTM::SendEntitiesData()
{
	while (threadRunning)
	{
		RLTM::SendSocketMessage(ENTITIES, entitiesData);
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

void RLTM::ResetDatas()
{
	RLTM::SetReplayAutoSave(false);
	threadRunning = false;
	isReplay = false;

	for (Event event : { MATCH, STATISTICS, ENTITIES })
		RLTM::SendSocketMessage(event, {});
}
