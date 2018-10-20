#include "stdafx.h"
#include "Kernel.h"
#include "Server.h"

constexpr int INSTANCE_EXISTED = 0xFFFFFFFF;
constexpr int INIT_FAILED = 0xFFFFFFFE;
constexpr int INIT_SUCCESS = 0x00000000;

static HSERVER g_hServer = nullptr;

int InitServer()
{
	if (g_hServer)
		return INSTANCE_EXISTED;

	g_hServer = new Server{};
	if (g_hServer->Init())
		return INIT_FAILED;

	return INIT_SUCCESS;
}

int RunServer()
{
	return g_hServer->Run();
}

std::string QueryServer(const char query[])
{
	std::string queryResult;
	if (std::strcmp(query, "show clients\n") == 0)
	{
		queryResult = 
			g_hServer->GetClients();
	}

	return queryResult;
}
