#include "EventController.h"

void EventController::HookEvents(GameWrapper* gameWrapper, WSC* wscInstance)
{
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerAdded", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TA.EventPlayerRemoved", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.AddLocalPlayer", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.PRI_TA.OnTeamChanged", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnBallHasBeenHit", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Team_TA.OnScoreUpdated", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function Engine.WorldInfo.EventPauseChanged", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", bind(&WSC::GetMatchData, wscInstance, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", bind(&WSC::SetReplayState, wscInstance, true, placeholders::_1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", bind(&WSC::SetReplayState, wscInstance, false, placeholders::_1));
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&WSC::GetPlayerStatData, wscInstance, placeholders::_1, placeholders::_2));
	gameWrapper->HookEventPost("Function Engine.GameViewportClient.Tick", bind(&WSC::GetEntitiesData, wscInstance));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&WSC::ResetDatas, wscInstance));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", bind(&WSC::SetSpectatorUI, wscInstance, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", bind(&WSC::SetSpectatorUI, wscInstance, 0));
	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", bind(&WSC::SetStatGraph, wscInstance));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay", bind(&WSC::SetReady, wscInstance));
}

void EventController::UnhookEvents(GameWrapper* gameWrapper)
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