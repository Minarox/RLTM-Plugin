#include "pch.h"
#include "RLTM.h"


BAKKESMOD_PLUGIN(RLTM, "Rocket League Tournament Manager plugin for BakkesMod", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

/*
	Public
*/
void RLTM::onLoad()
{
	_globalCvarManager = cvarManager;

	ix::initNetSystem();
	HookEvents();
	InitSocket();

	Log("RLTM Plugin loaded");

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	cvarManager->log("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&RLTM::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&RLTM::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	cvarManager->log("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RLTM::YourPluginMethod, this);
}

void RLTM::onUnload()
{
	socket.stop();
	UnhookEvents();
	ix::uninitNetSystem();

	Log("RLTM Plugin unloaded");
}

void RLTM::HookEvents()
{
	if (hooked) return;

	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState", std::bind(&RLTM::FetchGameData, this, 1));
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState", std::bind(&RLTM::FetchGameData, this, 0));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", std::bind(&RLTM::FetchGameData, this, 0));

	hooked = true;
}

void RLTM::UnhookEvents()
{
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.Ball_TA.Explode");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.BeginState");
	gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.EndState");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed");

	hooked = false;
}

void RLTM::InitSocket()
{
	if (socket.getReadyState() != ix::ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3000/?token=1234");
	socket.setHandshakeTimeout(4);
	socket.setPingInterval(45);
	socket.enableAutomaticReconnection();
	socket.setMinWaitBetweenReconnectionRetries(1000);
	socket.setMaxWaitBetweenReconnectionRetries(5000);

	socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
	{
		switch (msg->type)
		{
		case ix::WebSocketMessageType::Open:
			Log("Socket connected");
			break;

		case ix::WebSocketMessageType::Message:
			Log("Socket message: " + msg->str);
			break;

		case ix::WebSocketMessageType::Error:
			Log("Socket error: " + msg->errorInfo.reason);
			break;

		case ix::WebSocketMessageType::Close:
			Log("Socket disconnected");
			break;
		}
	});
	socket.start();
}

void RLTM::SendSocketMessage(std::string topic, json message)
{
	if (socket.getReadyState() != ix::ReadyState::Open) return;

	json data;
	data["topic"] = topic;
	data["message"] = message;

	socket.send(data.dump());
}

void RLTM::Log(std::string message)
{
	cvarManager->log(message);
}


/*
	Private
*/
void RLTM::FetchGameData(int isReplayingGoad)
{
	ServerWrapper localServer = gameWrapper->GetGameEventAsServer();
	ServerWrapper onlineServer = gameWrapper->GetOnlineGame();

	json message;

	if (!onlineServer.IsNull())
	{
		message["value"] = onlineServer.GetSecondsRemaining();
		message["score"] = GetGameScore(onlineServer);
		message["isOvertime"] = onlineServer.GetbOverTime();
		message["isReplay"] = isReplayingGoad;
	}
	else if (!localServer.IsNull())
	{
		message["value"] = localServer.GetSecondsRemaining();
		message["score"] = GetGameScore(localServer);
		message["isOvertime"] = localServer.GetbOverTime();
		message["isReplay"] = isReplayingGoad;
	}

	SendSocketMessage("game", message);
}

std::array<int, 2> RLTM::GetGameScore(ServerWrapper server)
{
	PlayerControllerWrapper serverLocalPrimaryPlayer = server.GetLocalPrimaryPlayer();
	ArrayWrapper<TeamWrapper> serverTeams = server.GetTeams();

	if (serverTeams.IsNull()) return { 0, 0 };

	if (!serverLocalPrimaryPlayer.IsNull())
		for (TeamWrapper team : serverTeams)
			if (serverLocalPrimaryPlayer.GetTeamNum2() != team.GetTeamNum2())
				return { serverTeams.Get(0).GetScore(), serverTeams.Get(1).GetScore() };

	return { serverTeams.Get(1).GetScore(), serverTeams.Get(0).GetScore() };
}