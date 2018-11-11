#include "stdafx.h"
#include "Kernel.h"
#include "Server.h"

typedef Server * HServer;
static HServer hServer = nullptr;
static Handle  SingleInstance = nullptr;

BOOL __stdcall InitServer()
{
	try
	{
		SingleInstance
			= CreateEvent(NULL, FALSE, FALSE, L"POKEMENSERVER");
		if (SingleInstance != NULL
			&& GetLastError() != ERROR_ALREADY_EXISTS)
		{
			if (hServer)
				throw std::exception("�������������С�\n");

			hServer = new Server{};
			if (hServer->Init())
				throw std::exception("��ʼ��������ʧ�ܡ�\n");
		}
	}
	catch (const std::exception& e)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL __stdcall RunServer()
{
	try
	{
		if (!hServer->Run())
			throw std::exception("����������ʧ�ܡ�\n");
	}
	catch (const std::exception& e)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL __stdcall IsServerOnRunning()
{
	return TRUE;
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
