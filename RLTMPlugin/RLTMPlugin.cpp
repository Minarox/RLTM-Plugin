#include "pch.h"
#include "RLTMPlugin.h"

BAKKESMOD_PLUGIN(RLTMPlugin, "Rocket League Tournament Manager Plugin", "1.0", PLUGINTYPE_SPECTATOR)

/*
	--- Public ---
*/
void RLTMPlugin::onLoad()
{
	cvarManager->executeCommand("plugin unload rocketstats; sleep 100; plugin unload autoreplayuploader; sleep 100; plugin unload rankviewer; sleep 100;", false);

	ix::initNetSystem();
	this->LoadWebSocket();

	// Statistique
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			StatTickerParams* pStruct = (StatTickerParams*)params;
			PriWrapper receiver = PriWrapper(pStruct->Receiver);
			StatEventWrapper statEvent = StatEventWrapper(pStruct->StatEvent);
			
			if (statEvent.GetEventName() == "Goal") playback_in_progress = true;

			json data;
			data["topic"] = "statistic";
			data["message"]["event_name"] = statEvent.GetEventName();
			data["message"]["name"] = receiver.GetPlayerName().ToString();
			data["message"]["uid"] = receiver.GetUniqueIdWrapper().GetIdString();
			data["message"]["score"] = receiver.GetMatchScore();
			data["message"]["ball_touches"] = receiver.GetBallTouches();
			data["message"]["car_touches"] = receiver.GetCarTouches();
			data["message"]["team_index"] = receiver.GetTeamNum();

			webSocket.send(data.dump());
		});

	// TODO: When a player car join the game
	// TODO: When a player car left the game

	// Pad de boost
	/*gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.VehiclePickup_Boost_TA.Pickup",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (playback_in_progress || !this->CheckValidGame()) return;
			webSocket.send("Boost pick");
		});*/

	// Touche du ballon
	/*gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.Car_TA.OnHitBall",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (playback_in_progress || !this->CheckValidGame()) return;
			webSocket.send("Ball hit");
		});*/

	// Vitesse supersonique
	/*gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.Car_TA.OnSuperSonicChanged",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (playback_in_progress || !this->CheckValidGame()) return;
			webSocket.send("Car supersonic changed");
		});*/

	// Pad de boost
	/*gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.VehiclePickup_Boost_TA.PlayPickedUpFX",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (playback_in_progress || !this->CheckValidGame()) return;
			webSocket.send("Car pick boost");
		});*/

	// Consommation du boost
	/*gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.CarComponent_Boost_TA.EventBoostAmountChanged",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (playback_in_progress || !this->CheckValidGame()) return;
			webSocket.send("Car boost");
		});*/

	// Ajout d'un joueur
	/*gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.AddCar",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
		});*/

	// Suppression d'un joueur
	/*gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.RemoveCar",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;

		});*/

	// Temps de jeu
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!game_in_progress || !this->CheckValidGame()) return;

			if (overtime_in_progress) game_time += 1;
			else game_time -= 1;
		
			total_game_time += 1;

			json data;
			data["topic"] = "time";
			data["message"]["overtime"] = overtime_in_progress;
			data["message"]["time"] = game_time;

			webSocket.send(data.dump());
		});

	// Temps de jeu (overtime)
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!game_in_progress || overtime_in_progress || !this->CheckValidGame()) return;

			overtime_in_progress = true;

			json data;
			data["topic"] = "overtime";

			webSocket.send(data.dump());
		});

	// Début de la partie
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function GameEvent_TA.Countdown.BeginState",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			playback_in_progress = false;
			overtime_in_progress = false;

			if (game_in_progress) return;
			game_in_progress = true;
			this->SetReplayAutoSave(true);

			ServerWrapper server = gameWrapper->GetCurrentGameState();
			if (server.IsNull()) return;

			json data;
			data["topic"] = "game_start";
			data["message"]["arena"] = gameWrapper->GetCurrentMap();
			data["message"]["time"] = server.GetGameTime();

			webSocket.send(data.dump());
			game_time = server.GetGameTime();
			total_game_time = 0;
		});

	// Fin de la partie
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			game_in_progress = false;
			playback_in_progress = true;

			json data;
			data["topic"] = "game_end";

			ServerWrapper server = gameWrapper->GetCurrentGameState();
			if (server.IsNull()) return;

			ArrayWrapper<TeamWrapper> teams = server.GetTeams();
			data["message"]["score_team_1"] = teams.Get(0).GetScore();
			data["message"]["score_team_2"] = teams.Get(1).GetScore();
			data["message"]["arena"] = gameWrapper->GetCurrentMap();
			data["message"]["duration"] = total_game_time;

			ArrayWrapper<PriWrapper> players = server.GetPRIs();
			int i = 0;

			for (int index = 0; index < players.Count(); index++) {
				PriWrapper player = players.Get(index);
				if (player.IsNull() || player.GetTeamNum() == 255) continue;

				data["message"]["statistic"][i]["name"] = player.GetPlayerName().ToString();
				data["message"]["statistic"][i]["uid"] = player.GetUniqueIdWrapper().GetIdString();
				data["message"]["statistic"][i]["score"] = player.GetMatchScore();
				data["message"]["statistic"][i]["ball_touches"] = player.GetBallTouches();
				data["message"]["statistic"][i]["car_touches"] = player.GetCarTouches();
				data["message"]["statistic"][i]["team_index"] = player.GetTeamNum();
				i++;
			}

			webSocket.send(data.dump());
		});

	// Replay de but (début)
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			playback_in_progress = true;

			json data;
			data["topic"] = "playback";
			data["message"] = true;

			webSocket.send(data.dump());
		});

	// Replay de but (fin)
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function GameEvent_Soccar_TA.ReplayPlayback.EndState",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			playback_in_progress = false;

			json data;
			data["topic"] = "playback";
			data["message"] = false;

			webSocket.send(data.dump());
		});

	// Temps forts (début)
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			playback_in_progress = true;

			json data;
			data["topic"] = "highlights";
			data["message"] = true;

			webSocket.send(data.dump());
			this->SetReady();
		});

	// Temps forts (fin)
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function ReplayDirector_TA.PlayingHighlights.Destroyed",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			if (!this->CheckValidGame()) return;
			playback_in_progress = false;

			json data;
			data["topic"] = "highlights";
			data["message"] = false;

			webSocket.send(data.dump());
		});

	// Changement de partie
	gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.GameEvent_Soccar_TA.Destroyed",
		[this](ServerWrapper caller, void* params, std::string eventname) {
			this->SetReplayAutoSave(false);
			game_in_progress = false;
			playback_in_progress = false;
			overtime_in_progress = false;
			game_time = 0;
			total_game_time = 0;

			json data;
			data["topic"] = "destroyed";

			webSocket.send(data.dump());
		});

	// Événements HUD
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx", std::bind(&RLTMPlugin::SetSpectatorUI, this, 100));
	gameWrapper->HookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD", std::bind(&RLTMPlugin::SetSpectatorUI, this, 0));
	gameWrapper->HookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs", std::bind(&RLTMPlugin::RemoveStatGraph, this));

	this->Log("--- RLTMPlugin loaded ---");
}

void RLTMPlugin::onUnload()
{
	gameWrapper->UnhookEventPost("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");
	//gameWrapper->UnhookEventPost("Function TAGame.Car_TA.OnSuperSonicChanged");
	//gameWrapper->UnhookEventPost("Function TAGame.Car_TA.OnHitBall");
	//gameWrapper->UnhookEventPost("Function TAGame.VehiclePickup_Boost_TA.PlayPickedUpFX");
	//gameWrapper->UnhookEventPost("Function TAGame.CarComponent_Boost_TA.EventBoostAmountChanged");
	//gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.AddCar");
	//gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.RemoveCar");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated");
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.BeginHighlightsReplay");
	gameWrapper->UnhookEvent("Function ReplayDirector_TA.PlayingHighlights.Destroyed");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");

	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.InitGFx");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_Spectator_TA.CycleHUD");
	gameWrapper->UnhookEvent("Function TAGame.StatGraphSystem_TA.GetDisplayGraphs");

	webSocket.stop();
	ix::uninitNetSystem();
	this->Log("--- RLTMPlugin unloaded ---");
}

/*
	--- Private ---
*/
void RLTMPlugin::LoadWebSocket()
{
	webSocket.setUrl("ws://localhost:9001/game");

	webSocket.setPingInterval(1);
	webSocket.start();

	webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
		{
			switch (msg->type)
			{
			case ix::WebSocketMessageType::Open:
				this->Log("WebSocket connected");
				break;
			case ix::WebSocketMessageType::Message:
				this->Log("Received message: " + msg->str);
				break;
			case ix::WebSocketMessageType::Error:
				this->Log("Error: " + msg->errorInfo.reason);
				break;
			case ix::WebSocketMessageType::Close:
				this->Log("WebSocket closed");
				break;
			}
		}
	);
}

void RLTMPlugin::SetSpectatorUI(int sleep)
{
	if (!this->CheckValidGame()) return;
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	PlayerControllerWrapper localPrimaryPlayerController = server.GetLocalPrimaryPlayer();
	if (localPrimaryPlayerController.IsNull()) return;

	PriWrapper localPrimaryPlayer = localPrimaryPlayerController.GetPRI();
	if (!localPrimaryPlayer.IsNull() && localPrimaryPlayer.IsSpectator()) 
		cvarManager->executeCommand("sleep " + std::to_string(sleep) + "; replay_gui hud 1; replay_gui names 1; replay_gui matchinfo 1; sleep 16; replay_gui hud 0; replay_gui names 1; replay_gui matchinfo 1", false);
}

void RLTMPlugin::RemoveStatGraph()
{
	EngineTAWrapper engine = gameWrapper->GetEngine();
	if (engine.IsNull()) return;

	StatGraphSystemWrapper statGraphs = engine.GetStatGraphs();
	if (!statGraphs.IsNull()) statGraphs.SetGraphLevel(6);
}

void RLTMPlugin::SetReplayAutoSave(bool status)
{
	cvarManager->executeCommand("ranked_autosavereplay_all " + std::to_string(status ? 1 : 0), false);
}

void RLTMPlugin::SetReady()
{
	ServerWrapper server = gameWrapper->GetOnlineGame();

	PlayerControllerWrapper playerController = gameWrapper->GetPlayerController();
	if (playerController.IsNull()) return;

	PriWrapper player = playerController.GetPRI();
	if (player.IsNull()) return;
	player.ServerReadyUp();
}

bool RLTMPlugin::CheckValidGame()
{
	ServerWrapper sw = gameWrapper->GetCurrentGameState();
	if (!sw) return false;

	GameSettingPlaylistWrapper playlist = sw.GetPlaylist();
	if (!playlist) return false;
	int playlistID = playlist.GetPlaylistId();
	return playlistID == 6;
}

void RLTMPlugin::Log(std::string message)
{
	cvarManager->log(message);
}
