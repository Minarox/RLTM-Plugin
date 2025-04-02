#include "WebSocketController.h"

void WebSocketController::InitSocket()
{
	CVarWrapper wsUrl = cvarManager->getCvar("wsc_url");

	if (socket.getReadyState() != ix::ReadyState::Closed || !wsUrl) return;

	socket.setUrl(wsUrl.getStringValue());
	socket.setHandshakeTimeout(3);
	socket.setPingInterval(1);
	socket.enableAutomaticReconnection();
	socket.setMinWaitBetweenReconnectionRetries(500);
	socket.setMaxWaitBetweenReconnectionRetries(500);
	socket.enablePerMessageDeflate();
	socket.addSubProtocol("json");

	socket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
		{
			switch (msg->type)
			{
				case ix::WebSocketMessageType::Open:
					for (auto& [key, value] : oldData.items())
					{
						json data = json::object();
						data["topic"] = key;
						data["payload"] = value;

						socket.send(data.dump());
					}
					break;
			}
		}
	);

	socket.start();
}

void WebSocketController::SendSocketMessage(Event event, json payload)
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