#include "stdafx.h"
#include "Kernel.h"
#include "Server.h"

constexpr int INSTANCE_EXISTED = 0xFFFFFFFF;
constexpr int INIT_FAILED = 0xFFFFFFFE;
constexpr int INIT_SUCCESS = 0x00000000;

typedef Server * HServer;
static HServer hServer = nullptr;

int InitServer()
{
	if (hServer)
		return INSTANCE_EXISTED;

	hServer = new Server{};
	if (hServer->Init())
		return INIT_FAILED;

	return INIT_SUCCESS;
}

int RunServer()
{
	return hServer->Run();
}

String QueryServer(const char query[])
{
	String queryResult;
	if (std::strcmp(query, "show clients\n") == 0)
	{
		queryResult = 
			hServer->GetClients();
	}

	return queryResult;
}
