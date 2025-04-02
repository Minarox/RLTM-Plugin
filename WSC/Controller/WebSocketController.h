#pragma once

#include "pch.h"

class WSC
{
public:
	void InitSocket();
	void SendSocketMessage(Event event, json payload);
};
