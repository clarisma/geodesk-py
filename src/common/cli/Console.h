// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <mutex>
#include <string_view>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif 
#include <common/text/Format.h>

/*
 * Console colors:
 * highlight
 * highlight2
 * progress bar
 * progress bar bground
 * progress percent
 * timestamp
 * success
 * error
 * warning
 * orange
 */
class Console
{
public:
	Console();
	~Console()
	{ 
		theConsole_ = nullptr; 
		restore();
	}

	void start(const char* task);
	void setTask(const char* task);
	void setProgress(int percentage);
	void log(std::string_view msg);
	void finish(std::string_view msg);
	
	template <size_t N>
	void log(const char(&msg)[N])
	{
		log(std::string_view(msg, N-1));  // Subtract 1 to exclude null terminator
	}

	void log(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		log(format, args);
		va_end(args);
	}

	void log(const char* format, va_list args)
	{
		char buf[1024];
		Format::unsafe(buf, format, args);
		log(std::string_view(buf));
	}

	static void msg(std::string_view msg)
	{
		if (theConsole_) theConsole_->log(msg);
	}

	template <size_t N>
	static void msg(const char(&msg)[N])
	{
		msg(std::string_view(msg, N - 1));  // Subtract 1 to exclude null terminator
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

	static void debug(const char* format, ...);

	static Console* get() { return theConsole_; }

	void print(const char* s, size_t len);

private:
	static const char* BLOCK_CHARS_UTF8;
	// static const int MAX_LINE_LENGTH = 78;
	static const int MAX_TASK_CHARS = 38;

	void init();
	void restore();
	
	static char* formatStatus(char* buf, int secs, int percentage = -1, const char* task = nullptr);
	static char* formatPercentage(char* buf, int percentage);
	static char* formatProgressBar(char* p, int percentage);
	size_t printWithStatus(char* buf, char* p, std::chrono::steady_clock::duration elapsed,
		int percentage = -1, const char* task = nullptr);

	static Console* theConsole_;
	#ifdef _WIN32
	HANDLE hConsole_;
	#endif
	std::atomic<const char*> currentTask_ = "";
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
	std::atomic<std::chrono::steady_clock::duration> reportNext_;
	std::atomic<int> currentPercentage_ = -1;
	int consoleWidth_ = 80;
};
