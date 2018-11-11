#pragma once

BOOL WINAPI InitServer();
BOOL WINAPI RunServer();

BOOL WINAPI IsServerOnRunning();

String QueryServer(const char[]);