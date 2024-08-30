#include "pch.h"
#include "RLTM.h"

void RLTM::SetSpectatorUI(int sleep)
{
	ServerWrapper server = RLTM::GetServerWrapper();
	if (!server) return;

	PlayerControllerWrapper primaryPlayer = server.GetLocalPrimaryPlayer();
	if (primaryPlayer.IsNull()) return;

	PriWrapper player = primaryPlayer.GetPRI();
	if (!player.IsNull() && player.IsSpectator())
	{
		//cvarManager->executeCommand("sleep " + to_string(sleep) + "; replay_gui hud 1; replay_gui names 1; replay_gui matchinfo 1; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
		cvarManager->executeCommand("sleep " + to_string(sleep) + "; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
	}
}

void RLTM::SetStatGraph()
{
	ServerWrapper server = RLTM::GetServerWrapper();
	if (!server) return;

	EngineTAWrapper engine = gameWrapper->GetEngine();
	if (engine.IsNull()) return;

	StatGraphSystemWrapper statGraphs = engine.GetStatGraphs();
	if (!statGraphs.IsNull()) statGraphs.SetGraphLevel(6);
}
