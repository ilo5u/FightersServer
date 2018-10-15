// PokemenEXEServer.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Kernel.h"

#pragma comment(lib, "WS2_32.lib")

int main(int argc, char *argv[])
{
	if (InitServer())
	{
		printf("Failed to initialize the server.\n");
		return 1;
	}

	if (RunServer())
	{
		printf("Failed to start the server.\n");
		return 2;
	}

	char szCmdLine[BUFSIZ];
	std::string szQueryResult;
	while (true)
	{
		printf("> ");
		fgets(szCmdLine, BUFSIZ, stdin);

		szQueryResult = QueryServer(szCmdLine);

		fprintf(stdout, szQueryResult.c_str());
		fprintf(stdout, "\n");
	}
    return 0;
}

