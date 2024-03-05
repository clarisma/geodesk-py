// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Console.h"
#define NOMINMAX
#include <windows.h>
#include <io.h> 


void Console::init()
{
	hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(hConsole_, &consoleMode);
	consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hConsole_, consoleMode);
	if (!SetConsoleOutputCP(CP_UTF8))
	{
		printf("Failed to enable UTF-8 support.\n");  // TODO
	}
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole_, &cursorInfo);
	cursorInfo.bVisible = false; // Set the cursor visibility
	SetConsoleCursorInfo(hConsole_, &cursorInfo);
}

void Console::print(const char* s, size_t len)
{
	DWORD written;
	WriteConsoleA(hConsole_, s, len, &written, NULL);
}

