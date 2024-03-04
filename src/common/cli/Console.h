// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <chrono>
#include <cstdint>
#include <string_view>
#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif 

class Console
{
public:
	Console();

	void start(int64_t totalWork, std::string_view initialTask);
	void progress(int64_t work);
	void setTask(std::string_view task);

private:
	static const char* BLOCK_CHARS_UTF8;
	static const int MAX_TASK_CHARS = 38;

	static void formatTimespan(char* buf, std::chrono::milliseconds ms);
	static void formatPercentage(char* buf, int percentage);
	static char* drawProgressBar(char* p, int percentage);

	HANDLE hConsole_;
	std::string_view currentTask_ = "";
	int64_t totalWork_ = 0;
	int64_t workCompleted_ = 0;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
	std::chrono::time_point<std::chrono::steady_clock> nextReportTime_;
	int percentageCompleted_ = -1;
	int statusLen_ = 0;
	char status_[256];
};
