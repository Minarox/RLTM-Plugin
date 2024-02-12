#include "pch.h"
#include "DSCSPlugin.h"

BAKKESMOD_PLUGIN(DSCSPlugin, "DawaEsport Championship Plugin", "1.0", PERMISSION_ALL)

/*
	--- Public ---
*/
void DSCSPlugin::onLoad()
{
	this->Log("--- DSCSPlugin loaded ---");
}

void DSCSPlugin::onUnload()
{
	this->Log("--- DSCSPlugin unloaded ---");
}

/*
	--- Private ---
*/
void DSCSPlugin::Log(std::string message)
{
	cvarManager->log(message);
}