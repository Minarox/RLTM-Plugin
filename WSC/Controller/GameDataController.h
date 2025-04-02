#pragma once

#include "pch.h"

class WSC
{
public:
	ServerWrapper GetServerWrapper();
	void GetMatchData(string caller);
	json GetScore(ServerWrapper server);
	json GetStatistics(ServerWrapper server);
	void GetPlayerStatData(ServerWrapper _server, void* params);
	void GetEntitiesData();
	void SendEntitiesData();
	void GetPlayersData(ServerWrapper server);
	void ResetDatas();
};
