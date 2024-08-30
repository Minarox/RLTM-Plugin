#include "pch.h"
#include "RLTM.h"

void RLTM::SetReplayAutoSave(bool status)
{
	if (autoSaveReplay == status) return;
	autoSaveReplay = status;
	cvarManager->executeCommand("ranked_autosavereplay_all " + to_string(status ? 1 : 0), false);
}
