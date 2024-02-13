#include "pch.h"
#include "DSCSPlugin.h"

BAKKESMOD_PLUGIN(DSCSPlugin, "DawaEsport Championship Plugin", "1.0", PERMISSION_ALL)

/*
	--- Public ---
*/
void DSCSPlugin::onLoad()
{
	ix::initNetSystem();
	this->LoadWebSocket();

	this->Log("--- DSCSPlugin loaded ---");
}

void DSCSPlugin::onUnload()
{
	webSocket.stop();
	ix::uninitNetSystem();
	this->Log("--- DSCSPlugin unloaded ---");
}

/*
	--- Private ---
*/
void DSCSPlugin::LoadWebSocket()
{
	webSocket.setUrl("ws://localhost:9001/game");

	webSocket.setPingInterval(1);
	webSocket.start();

	webSocket.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg)
		{
			switch (msg->type)
			{
			case ix::WebSocketMessageType::Open:
				this->Log("WebSocket connected");
				webSocket.send("hello world");
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

void DSCSPlugin::Log(std::string message)
{
	cvarManager->log(message);
}
