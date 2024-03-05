// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string_view>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif 
#include <common/text/Format.h>

class Console
{
public:
	Console();
	~Console() { theConsole_ = nullptr; }

	void start(int64_t totalWork, std::string_view initialTask);
	void progress(int64_t work);
	void setTask(std::string_view task);
	void log(std::string_view msg);
	
	template <size_t N>
	void log(const char(&msg)[N])
	{
		log(std::string_view(msg, N-1));  // Subtract 1 to exclude null terminator
	}

	void log(const char* format, ...)
	{
		char buf[1024];
		va_list args;
		va_start(args, format);
		Format::unsafe(buf, format, args);
		va_end(args);
		log(std::string_view(buf));
	}

	static void msg(std::string_view msg)
	{
		if (theConsole_) theConsole_->log(msg);
	}

	template <size_t N>
	static void msg(const char(&msg)[N])
	{
		LOG(std::string_view(msg, N - 1));  // Subtract 1 to exclude null terminator
	}


	static void msg(const char* format, ...)
	{
		if (theConsole_)
		{
			va_list args;
			va_start(args, format);
			theConsole_->log(format, args);
			va_end(args);
		}
	}

private:
	static const char* BLOCK_CHARS_UTF8;
	static const int MAX_LINE_LENGTH = 78;
	static const int MAX_TASK_CHARS = 38;

	void init();
	void print(const char* s, size_t len);

	static void formatTimespan(char* buf, std::chrono::milliseconds ms, bool withMs = false);
	static void formatPercentage(char* buf, int percentage);
	static char* drawProgressBar(char* p, int percentage);

	static Console* theConsole_;
	#ifdef _WIN32
	HANDLE hConsole_;
	#endif
	std::mutex mutex_;
	std::string_view currentTask_ = "";
	int64_t totalWork_ = 0;
	int64_t workCompleted_ = 0;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
	std::chrono::time_point<std::chrono::steady_clock> nextReportTime_;
	int percentageCompleted_ = -1;
	int statusLen_ = 0;
	char status_[256];
};
