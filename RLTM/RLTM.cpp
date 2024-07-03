#include "pch.h"
#include "RLTM.h"


BAKKESMOD_PLUGIN(RLTM, "Rocket League Tournament Manager plugin for BakkesMod", plugin_version, PLUGINTYPE_SPECTATOR)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void RLTM::onLoad()
{
	_globalCvarManager = cvarManager;

	ix::initNetSystem();
	HookEvents();
	initSocket();

	cvarManager->log("RLTM Plugin loaded");

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

	cvarManager->log("RLTM Plugin unloaded");
}

void RLTM::HookEvents()
{
	if (hooked) return;

	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RLTM::YourPluginMethod, this);

	hooked = true;
}

void RLTM::UnhookEvents()
{
	//gameWrapper->UnhookEvent("Function TAGame.Ball_TA.Explode", std::bind(&RLTM::YourPluginMethod, this);

	hooked = false;
}

void RLTM::InitSocket()
{
	if (socket.getReadyState() !== ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3000/");
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
			cvarManager->log("Socket connected");
			break;
		case ix::WebSocketMessageType::Message:
			cvarManager->log("Socket message: " + msg->str);
			break;
		case ix::WebSocketMessageType::Error:
			cvarManager->log("Socket errro: " + msg->errorInfo.reason);
			break;
		case ix::WebSocketMessageType::Close:
			cvarManager->log("Socket disconnected");
			break;
		}
	});
	socket.start();
}