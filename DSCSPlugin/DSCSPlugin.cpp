#include "pch.h"
#include "DSCSPlugin.h"
#include <iostream>

BAKKESMOD_PLUGIN(DSCSPlugin, "DawaEsport Championship Plugin", "1.0", PERMISSION_ALL)

void DSCSPlugin::onLoad()
{
	// Connection au serveur
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.AddGameBall", std::bind(&DSCSPlugin::JoinSpectator, this));
	gameWrapper->HookEvent("Function TAGame.GFxData_LocalPlayer_TA.Spectate", std::bind(&DSCSPlugin::SetSpectatorUI, this, 400));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", std::bind(&DSCSPlugin::SetSpectatorUI, this, 0));
	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", std::bind(&DSCSPlugin::RemoveStatGraph, this));
	
	// Début du match
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&DSCSPlugin::ShowHUD, this));

	// Maj HUD stream
	// gameWrapper->HookEvent("Temps", std::bind(&DSCSPlugin::AddGoal, this));
	// gameWrapper->HookEvent("But d'une équipe", std::bind(&DSCSPlugin::AddGoal, this));
	// gameWrapper->HookEvent("Etat du boost d'un joueur", std::bind(&DSCSPlugin::AddGoal, this));
	// gameWrapper->HookEvent("Replay d'un but", std::bind(&DSCSPlugin::AddGoal, this));

	// Fin du match
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", std::bind(&DSCSPlugin::MatchEnded, this));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.Destroyed", std::bind(&DSCSPlugin::HideHUD, this));
}

void DSCSPlugin::onUnload()
{
	// Connection au serveur
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.AddGameBall");
	gameWrapper->UnhookEvent("Function TAGame.GFxData_LocalPlayer_TA.Spectate");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");
	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");
	
	// Début du match
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");

	// MAJ HUD stream

	// Fin du match
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.Destroyed");
}

void DSCSPlugin::JoinSpectator()
{
	if (!gameWrapper->IsInOnlineGame() || matchInProgress) return;
	this->Log("========= JoinSpectator =========");

	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (server.IsNull()) return;

	GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	if (!playlist.IsPrivateMatch() && !playlist.IsLanMatch()) return;

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	playerController.Spectate();

	this->RemoveStatGraph();
	this->SetSpectatorUI(400);
}

void DSCSPlugin::SetSpectatorUI(int sleep)
{
	if (!gameWrapper->IsInOnlineGame()) return;
	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (server.IsNull()) return;	
	
	GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	if (!playlist.IsPrivateMatch() && !playlist.IsLanMatch()) return;

	this->Log("========= SetSpectatorUI =========");
	cvarManager->executeCommand("sleep " + std::to_string(sleep) + "; replay_gui hud 1; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
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

void DSCSPlugin::ShowHUD()
{
	if (!gameWrapper->IsInOnlineGame() || matchInProgress) return;
	matchInProgress = true;
	this->Log("========= ShowHUD =========");
}

void DSCSPlugin::MatchEnded()
{
	if (!gameWrapper->IsInOnlineGame() || !matchInProgress) return;
	matchInProgress = false;
	this->Log("========= MatchEnded =========");
	//std::map<std::string, std::string> matchValues;

	//if (!gameWrapper->IsInOnlineGame()) return;
	//ServerWrapper server = gameWrapper->GetOnlineGame();
	//if (server.IsNull()) return;
	//	
	//GameSettingPlaylistWrapper playlist = server.GetPlaylist();
	//if (!playlist.IsPrivateMatch() && !playlist.IsLanMatch()) return;

	//PlayerControllerWrapper localPrimaryPlayerController = server.GetLocalPrimaryPlayer();
	//if (localPrimaryPlayerController.IsNull()) return;

	//PriWrapper localPrimaryPlayer = localPrimaryPlayerController.GetPRI();
	//if (localPrimaryPlayer.IsNull()) return;

	//TeamWrapper matchWinner = server.GetMatchWinner();
	//if (matchWinner.IsNull()) return;

	//matchValues.clear();

	//time_t now = time(NULL);
	//tm nowInfo;
	//localtime_s(&nowInfo, &now);

	//char timestamp[32];
	//strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &nowInfo);
	//int playlistId = playlist.GetPlaylistId();

	//matchValues["Timestamp"] = std::string(timestamp);
	//matchValues["Win"] = std::to_string((localPrimaryPlayer.GetTeamNum() == matchWinner.GetTeamNum()));
	//matchValues["Playlist ID"] = std::to_string(playlistId);
	//matchValues["Playlist"] = playlist.GetTitle().ToString();
	//matchValues["Ranked"] = std::to_string(playlist.GetbRanked());
	//matchValues["Goals"] = std::to_string(localPrimaryPlayer.GetMatchGoals());
	//matchValues["Assists"] = std::to_string(localPrimaryPlayer.GetMatchAssists());
	//matchValues["Saves"] = std::to_string(localPrimaryPlayer.GetMatchSaves());
	//matchValues["Demos"] = std::to_string(localPrimaryPlayer.GetMatchDemolishes());
	//matchValues["Shots"] = std::to_string(localPrimaryPlayer.GetMatchShots());
	//matchValues["Score"] = std::to_string(localPrimaryPlayer.GetMatchScore());
	//matchValues["MVP"] = std::to_string(localPrimaryPlayer.GetbMatchMVP());

	//// Afficher les valeurs dans la console
	//for (const auto& pair : matchValues) {
	//	this->Log(("%s: %s\n", pair.first.c_str(), pair.second.c_str()));
	//}
}

void DSCSPlugin::HideHUD()
{
	if (!matchInProgress) return;
	matchInProgress = false;
	this->Log("========= HideHUD =========");
}

void DSCSPlugin::Log(std::string msg)
{
	cvarManager->log(msg);
}