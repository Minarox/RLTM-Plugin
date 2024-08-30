#include "pch.h"
#include "RLTM.h"

void RLTM::InitSocket()
{
	if (socket.getReadyState() != ix::ReadyState::Closed) return;

	socket.setUrl("ws://localhost:3000?token=457218CD39CB96BF59086E83C4D21AC6");
	socket.setHandshakeTimeout(4);
	socket.setPingInterval(2);
	socket.enableAutomaticReconnection();
	socket.setMinWaitBetweenReconnectionRetries(1000);
	socket.setMaxWaitBetweenReconnectionRetries(5000);
	socket.enablePerMessageDeflate();
	socket.addSubProtocol("json");

	socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
		{
			switch (msg->type)
			{
			case ix::WebSocketMessageType::Open:
				cvarManager->log("Socket connected");
				for (auto& [key, value] : oldData.items())
				{
					json data = json::object();
					data["topic"] = key;
					data["payload"] = value;

					socket.send(data.dump());
				}
				break;

			case ix::WebSocketMessageType::Message:
				cvarManager->log("Socket message: " + msg->str);
				break;

			case ix::WebSocketMessageType::Error:
				cvarManager->log("Socket error: " + msg->errorInfo.reason);
				break;

			case ix::WebSocketMessageType::Close:
				cvarManager->log("Socket disconnected");
				break;
			}
		});
	socket.start();
}

void RLTM::StopSocket()
{
    socket.stop();
}

void RLTM::SendSocketMessage(Event event, json payload)
{
	string topic = eventToTopic[event];

	json data = json::object();
	data["topic"] = topic;
	data["payload"] = payload;

	if (data["payload"] == oldData[topic]) return;
	if (topic != eventToTopic[STATISTIC]) oldData[topic] = data["payload"];

	if (socket.getReadyState() != ix::ReadyState::Open) return;
	socket.send(data.dump());
}
