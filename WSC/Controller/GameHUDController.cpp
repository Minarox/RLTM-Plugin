#include "GameHUDController.h"

void GameHUDController::SetSpectatorUI(int sleep, shared_ptr<GameWrapper> gameWrapper, shared_ptr<CVarManagerWrapper> cvarManager)
{
	ServerWrapper server = GetServerWrapper(gameWrapper);
	if (!server) return;

	PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
	if (!primaryPlayer) return;

	PriWrapper player = primaryPlayer.GetPRI();
	if (player)
	{
		if (player.IsSpectator())
		{
			cvarManager->executeCommand("sleep " + to_string(sleep) + "; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
		}
	}
}

void GameHUDController::SetStatGraph(shared_ptr<GameWrapper> gameWrapper)
{
	ServerWrapper server = GetServerWrapper(gameWrapper);
	if (!server) return;

	EngineTAWrapper engine = gameWrapper->GetEngine();
	if (!engine) return;

	StatGraphSystemWrapper statGraphs = engine.GetStatGraphs();
	if (statGraphs) statGraphs.SetGraphLevel(6);
}

void GameHUDController::SetReady(shared_ptr<GameWrapper> gameWrapper)
{
	ServerWrapper server = GetServerWrapper(gameWrapper);
	if (!server) return;

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (!playerController) return;

	PriWrapper player = playerController.GetPRI();
	if (player) player.ServerReadyUp();
}

void GameHUDController::SetSpectator(shared_ptr<GameWrapper> gameWrapper)
{
	ServerWrapper server = GetServerWrapper(gameWrapper);
	if (!server) return;

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (!playerController) return;

	PriWrapper player = playerController.GetPRI();
	if (player)
	{
		if (player.IsPlayer()) player.ServerSpectate();
	}
}

ServerWrapper GameHUDController::GetServerWrapper(shared_ptr<GameWrapper> gameWrapper)
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