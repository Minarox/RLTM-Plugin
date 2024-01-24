#include "pch.h"
#include "DSCSPlugin.h"
#include <iostream>

BAKKESMOD_PLUGIN(DSCSPlugin, "DawaEsport Championship Plugin", "1.0", PERMISSION_ALL)

/*
	--- Public ---
*/
void DSCSPlugin::onLoad()
{
	// Etats du match
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&DSCSPlugin::UpdateMatchStatus, this, true));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&DSCSPlugin::UpdatePlaybackStatus, this, true));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", std::bind(&DSCSPlugin::UpdatePlaybackStatus, this, false));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", std::bind(&DSCSPlugin::UpdateMatchStatus, this, false));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", std::bind(&DSCSPlugin::UpdateHighlightStatus, this, true));
	gameWrapper->HookEvent("Function ReplayDirector_TA.PlayingHighlights.Destroyed", std::bind(&DSCSPlugin::UpdateHighlightStatus, this, false));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&DSCSPlugin::ResetStatus, this));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.Destroyed", std::bind(&DSCSPlugin::ResetStatus, this));

	// Actions par defaut
	gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.AddGameBall", std::bind(&DSCSPlugin::JoinSpectator, this));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", std::bind(&DSCSPlugin::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", std::bind(&DSCSPlugin::SetSpectatorUI, this, 0));
	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", std::bind(&DSCSPlugin::RemoveStatGraph, this));

	if (this->IsGameValid()) {
		this->RemoveStatGraph();

		ServerWrapper server = gameWrapper->GetOnlineGame();
		PlayerControllerWrapper localPrimaryPlayerController = server.GetLocalPrimaryPlayer();
		if (localPrimaryPlayerController.IsNull()) return;

		PriWrapper localPrimaryPlayer = localPrimaryPlayerController.GetPRI();
		if (localPrimaryPlayer.IsNull() || localPrimaryPlayer.IsSpectator()) return;
		
		this->JoinSpectator();
	}
	
	try {
		socket.connect("ws://localhost:3000");
	}
	catch (std::exception& e) {
		this->Log("========= Unable to connect to Back-End =========");
	}
}

void DSCSPlugin::onUnload()
{
	this->Log(std::to_string(socket.opened()));
	socket.close();

	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");
	gameWrapper->UnhookEvent("Function ReplayDirector_TA.PlayingHighlights.Destroyed");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.Destroyed");

	gameWrapper->UnhookEventPost("Function TAGame.GameEvent_Soccar_TA.AddGameBall");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");
	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");

	this->ResetStatus();
}

void DSCSPlugin::JoinSpectator()
{
	if (!this->IsGameValid() || matchStatus || playbackStatus || highlightStatus) return;
	ServerWrapper server = gameWrapper->GetOnlineGame();

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	this->Log("========= JoinSpectator =========");
	playerController.Spectate();
}

void DSCSPlugin::SetSpectatorUI(int sleep)
{
	if (!this->IsGameValid()) return;
	ServerWrapper server = gameWrapper->GetOnlineGame();
	PlayerControllerWrapper localPrimaryPlayerController = server.GetLocalPrimaryPlayer();
	if (localPrimaryPlayerController.IsNull()) return;

	PriWrapper localPrimaryPlayer = localPrimaryPlayerController.GetPRI();
	if (localPrimaryPlayer.IsNull() || !localPrimaryPlayer.IsSpectator()) return;

	this->Log("========= SetSpectatorUI =========");
	cvarManager->executeCommand("sleep " + std::to_string(sleep) + "; replay_gui hud 1; replay_gui names 1; replay_gui matchinfo 1; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
}

void DSCSPlugin::RemoveStatGraph()
{
	EngineTAWrapper engine = gameWrapper->GetEngine();
	if (engine.IsNull()) return;

	StatGraphSystemWrapper statGraphs = engine.GetStatGraphs();
	if (statGraphs.IsNull()) return;

	this->Log("========= RemoveStatGraph =========");
	statGraphs.SetGraphLevel(6);
}

void DSCSPlugin::FetchStats()
{
	if (!this->IsGameValid()) return;
	ServerWrapper server = gameWrapper->GetOnlineGame();

	this->Log("========= MatchEnded =========");
	json data;
	std::map<std::string, std::string> matchValues;
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	
	data["score_team_1"] = teams.Get(0).GetScore();
	data["score_team_2"] = teams.Get(1).GetScore();
	data["arena"] = gameWrapper->GetCurrentMap();
	data["duration"] = match_ended_at - match_started_at;

	ArrayWrapper<PriWrapper> players = server.GetPRIs();
	for (int i = 0; i < players.Count(); i++) {
		PriWrapper player = players.Get(i);
		if (player.IsNull() || player.GetTeamNum() == 255) continue;

		data["stats"][i]["name"] = player.GetPlayerName().ToString();
		data["stats"][i]["id"] = player.GetUniqueIdWrapper().GetIdString();
		data["stats"][i]["uid"] = player.GetUniqueIdWrapper().GetUID();
		data["stats"][i]["team"] = player.GetTeamNum();
		data["stats"][i]["score"] = player.GetMatchScore();
		data["stats"][i]["goals"] = player.GetMatchGoals();
		data["stats"][i]["own_goals"] = player.GetMatchOwnGoals();
		data["stats"][i]["assists"] = player.GetMatchAssists();
		data["stats"][i]["saves"] = player.GetMatchSaves();
		data["stats"][i]["shots"] = player.GetMatchShots();
		data["stats"][i]["demolishes"] = player.GetMatchDemolishes();
		data["stats"][i]["bonus_xp"] = player.GetMatchBonusXP();
		data["stats"][i]["damage"] = player.GetMatchBreakoutDamage();
		data["stats"][i]["mvp"] = player.GetbMatchMVP();
		data["stats"][i]["boost_pickups"] = player.GetBoostPickups();
		data["stats"][i]["ball_touches"] = player.GetBallTouches();
		data["stats"][i]["car_touches"] = player.GetCarTouches();
	}

	if (socket.opened()) {
		socket.socket()->emit("match_ended", data.dump());
	}
}

void DSCSPlugin::SetReplayAutoSave(bool status)
{
	if (status) {
		if (this->IsGameValid()) cvarManager->executeCommand("ranked_autosavereplay_all 1", false);
	}
	else {
		cvarManager->executeCommand("ranked_autosavereplay_all 0", false);
	}
}

void DSCSPlugin::ReadyUp()
{
	if (!this->IsGameValid()) return;
	ServerWrapper server = gameWrapper->GetOnlineGame();

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	PriWrapper player = playerController.GetPRI();
	if (player.IsNull()) return;
	
	this->Log("========= SetReady =========");
	player.ServerReadyUp();
}

/*
	--- Private ---
*/
bool DSCSPlugin::IsGameValid()
{
	if (!gameWrapper->IsInOnlineGame()) return false;
	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (server.IsNull()) return false;

	GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	if (!playlist.IsPrivateMatch() && !playlist.IsLanMatch()) return false;

	return true;
}

void DSCSPlugin::UpdateMatchStatus(bool status)
{
	if (matchStatus == status) return;
	matchStatus = status;
	this->Log(("========= matchStatus " + std::to_string(matchStatus) + " =========").c_str());

	if (socket.opened()) {
		socket.socket()->emit("match_status", std::to_string(matchStatus));
	}

	if (matchStatus) {
		/* Match en cours */
		match_started_at = std::time(0);
		this->SetReplayAutoSave(true);
		// Montrer HUD stream
	}
	else {
		/* Match termine */
		match_ended_at = std::time(0);
		this->FetchStats();
		// Cacher HUD stream
	}
}

void DSCSPlugin::UpdatePlaybackStatus(bool status)
{
	if (playbackStatus == status) return;
	if (status && !matchStatus) this->UpdateMatchStatus(true);
	playbackStatus = status;
	this->Log(("========= playbackStatus " + std::to_string(playbackStatus) + " =========").c_str());

	if (socket.opened()) {
		socket.socket()->emit("playback_status", std::to_string(playbackStatus));
	}

	if (playbackStatus) {
		/* Replay du but en cours */
		// Montrer "Replay" HUD stream
	}
	else {
		/* Replay du but termine */
		// Cacher "Replay" HUD stream
	}
}

void DSCSPlugin::UpdateHighlightStatus(bool status)
{
	if (highlightStatus == status) return;
	highlightStatus = status;
	this->Log(("========= highlightStatus " + std::to_string(highlightStatus) + " =========").c_str());

	if (socket.opened()) {
		socket.socket()->emit("highlight_status", std::to_string(highlightStatus));
	}

	if (highlightStatus) {
		/* Replay de fin de match en cours */
		this->ReadyUp();
	}
	else {
		/* Replay de fin de match termine */
		this->SetReplayAutoSave(false);
	}
}

void DSCSPlugin::ResetStatus()
{
	this->Log("========= ResetStatus =========");

	if (socket.opened()) {
		socket.socket()->emit("reset_status");
	}

	matchStatus = false;
	playbackStatus = false;
	highlightStatus = false;
	this->SetReplayAutoSave(false);
}

void DSCSPlugin::Log(std::string message)
{
	cvarManager->log(message);

	if (socket.opened()) {
		socket.socket()->emit("log", message);
	}
}