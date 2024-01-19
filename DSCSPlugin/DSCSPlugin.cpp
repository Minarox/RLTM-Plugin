#include "pch.h"
#include "DSCSPlugin.h"
#include <iostream>

BAKKESMOD_PLUGIN(DSCSPlugin, "DawaEsport Championship Plugin", "1.0", PERMISSION_ALL)

/*
	--- Public ---
*/
void DSCSPlugin::onLoad()
{
	// États du match
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&DSCSPlugin::UpdateMatchStatus, this, true));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&DSCSPlugin::UpdatePlaybackStatus, this, true));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", std::bind(&DSCSPlugin::UpdatePlaybackStatus, this, false));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", std::bind(&DSCSPlugin::UpdateMatchStatus, this, false));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", std::bind(&DSCSPlugin::UpdateHighlightStatus, this, true));
	gameWrapper->HookEvent("Function ReplayDirector_TA.PlayingHighlights.Destroyed", std::bind(&DSCSPlugin::UpdateHighlightStatus, this, false));

	// Actions par défaut
	gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.AddGameBall", std::bind(&DSCSPlugin::JoinSpectator, this));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", std::bind(&DSCSPlugin::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", std::bind(&DSCSPlugin::SetSpectatorUI, this, 0));
	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", std::bind(&DSCSPlugin::RemoveStatGraph, this));

	/*
		Si la partie est en cours, matchInProgress = true et matchEnded = false et si on est pas en spectateur, rejoindre en tant que spectateur et supprimer l'overlay spectateur et le graphique des statistiques
	*/
}

void DSCSPlugin::onUnload()
{
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");
	gameWrapper->UnhookEvent("Function ReplayDirector_TA.PlayingHighlights.Destroyed");

	gameWrapper->UnhookEventPost("Function TAGame.GameEvent_Soccar_TA.AddGameBall");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");
	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");
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

	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	this->Log(("Score: Bleu (" + std::to_string(teams.Get(0).GetScore()) + ") - Orange (" + std::to_string(teams.Get(1).GetScore()) + ")").c_str());

	ArrayWrapper<PriWrapper> players = server.GetPRIs();
	for (int i = 0; i < players.Count(); i++) {
		PriWrapper player = players.Get(i);
		if (player.IsNull() || player.GetTeamNum() == 255) continue;

		this->Log("========= Stat =========");
		this->Log(("Player " + player.GetPlayerName().ToString()).c_str());
		this->Log(("Player ID " + player.GetUniqueIdWrapper().GetIdString()).c_str());
		this->Log(("Player UID " + std::to_string(player.GetUniqueIdWrapper().GetUID())).c_str());
		this->Log(("Team " + std::to_string(player.GetTeamNum())).c_str());
		this->Log(("Score " + std::to_string(player.GetMatchScore())).c_str());
		this->Log(("Goals " + std::to_string(player.GetMatchGoals())).c_str());
		this->Log(("Own goals " + std::to_string(player.GetMatchOwnGoals())).c_str());
		this->Log(("Assists " + std::to_string(player.GetMatchAssists())).c_str());
		this->Log(("Saves " + std::to_string(player.GetMatchSaves())).c_str());
		this->Log(("Shots " + std::to_string(player.GetMatchShots())).c_str());
		this->Log(("Demolishes " + std::to_string(player.GetMatchDemolishes())).c_str());
		this->Log(("Bonus XP " + std::to_string(player.GetMatchBonusXP())).c_str());
		this->Log(("Damage " + std::to_string(player.GetMatchBreakoutDamage())).c_str());
		this->Log(("MVP " + std::to_string(player.GetbMatchMVP())).c_str());
		this->Log(("Boost Pickups " + std::to_string(player.GetBoostPickups())).c_str());
		this->Log(("Ball Touches " + std::to_string(player.GetBallTouches())).c_str());
		this->Log(("Car Touches " + std::to_string(player.GetCarTouches())).c_str());
	}
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
	this->Log(("========= matchStatus " + std::to_string(status) + " =========").c_str());

	if (matchStatus) {
		// Montrer HUD stream
	}
	else {
		this->FetchStats();
		// Cacher HUD stream
	}
}

void DSCSPlugin::UpdatePlaybackStatus(bool status)
{
	if (playbackStatus == status) return;
	playbackStatus = status;
	this->Log(("========= playbackStatus " + std::to_string(status) + " =========").c_str());

	if (playbackStatus) {
		// Montrer HUD stream "Replay"
	}
	else {
		// Cacher HUD stream "Replay"
	}
}

void DSCSPlugin::UpdateHighlightStatus(bool status)
{
	if (highlightStatus == status) return;
	highlightStatus = status;
	this->Log(("========= highlightStatus " + std::to_string(status) + " =========").c_str());

	if (highlightStatus) {
		// Sauvegarder replay
		// Se mettre prêt pour le prochain match
	}
}

void DSCSPlugin::ResetStatus()
{
	matchStatus = false;
	playbackStatus = false;
	highlightStatus = false;
}

void DSCSPlugin::Log(std::string msg)
{
	cvarManager->log(msg);
}