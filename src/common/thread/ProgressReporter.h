// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdio.h>
#include <common/text/Format.h>

#include <windows.h>
#include <fcntl.h>   // TODO: Windows only
#include <io.h>     // For _setmode() and _fileno()
// TODO: MSVC only
#pragma execution_character_set("utf-8")

class ProgressReporter
{
public:
	ProgressReporter() :
		totalUnits_(0),
		unitsCompleted_(0),
		percentageCompleted_(0)
	{
		memset(display_, ' ', 80);
		int crPos = TASK_START + taskWidth_ + PROGRESS_BAR_WIDTH + PERCENTAGE_WIDTH;
		display_[crPos] = '\r';
		display_[crPos + 1] = 0;
		setBlockChar(&display_[TASK_START + taskWidth_], PROGRESS_DELIMITER_LEFT);
		setBlockChar(&display_[TASK_START + taskWidth_ + PROGRESS_BAR_WIDTH - 3], 
			PROGRESS_DELIMITER_RIGHT);
	}

	void setTask(const char* task)
	{
		memset(&display_[TASK_START], ' ', taskWidth_);
		memcpy(&display_[TASK_START], task, std::min(strlen(task), 
			static_cast<size_t>(taskWidth_)));
	}

	void start(uint64_t totalUnits)
	{
		totalUnits_ = totalUnits;
		startTime_ = std::chrono::steady_clock::now();
		lastReportTime_ = startTime_;
		report();
	}

	void progress(uint64_t units)
	{
		unitsCompleted_ += units;
		int percentage = (int)(unitsCompleted_ * 100 / totalUnits_);
		if (percentage != percentageCompleted_)
		{
			std::chrono::time_point<std::chrono::steady_clock> now(
				std::chrono::steady_clock::now());
			// Check if at least one second has elapsed
			if (now - lastReportTime_ >= std::chrono::seconds(1))
			{
				lastReportTime_ = now;
				percentageCompleted_ = percentage;
				report();
			}
		}
	}

	void report()
	{
		int fullBlockCount = percentageCompleted_ / 4;
		char* p = &display_[TASK_START + taskWidth_ + 3];
		char* pEnd = p + fullBlockCount * 3;
		while(p < pEnd)
		{
			setBlockChar(p, PROGRESS_FULL_BLOCK);
			p += 3;
		}
		int quarterBlockCount = percentageCompleted_ % 4;
		if (quarterBlockCount)
		{
			setBlockChar(pEnd, PROGRESS_PARTIAL_BLOCKS[quarterBlockCount]);
		}
		sprintf(&display_[TASK_START + taskWidth_ + PROGRESS_BAR_WIDTH],
			"%d%%", percentageCompleted_);
		const char* xxx = (const char*)u8"straße 世界▕████████    \n";

		fflush(stdout);
		SetConsoleOutputCP(CP_UTF8);
		_setmode(_fileno(stdout), _O_U8TEXT);
		fflush(stdout);

		printf("monkey\n");
		printf("straße\n");
		printf("世界\n");
		printf("▕█\n");
		printf("%s", xxx);
		printf("%s", display_);
		fflush(stdout);
		// printf("%s... %d%%\r", verb_, percentageCompleted_);
	}

	void end(const char* what)
	{
		std::chrono::time_point<std::chrono::steady_clock> now(
			std::chrono::steady_clock::now());
		char buf[32];
		Format::timespan(buf, std::chrono::duration_cast
			<std::chrono::milliseconds>(now - startTime_));
		printf("%s in %s\n", what, buf);
	}

private:
	static const int TASK_START = 10;
	static const int MAX_TASK_WIDTH = 35;
	static const int PROGRESS_BAR_WIDTH = 81;
		// 25 block chars + 2 delimiters = 27, with 3 encoded chars each
	static const int PERCENTAGE_WIDTH = 4;
	static const int DISPLAY_BUF_SIZE = 
		TASK_START + MAX_TASK_WIDTH + PROGRESS_BAR_WIDTH + 20;

	static inline const char* TEST_CARS = "世界";

	static inline const char* BLOCK_CHARS_UTF8 = 
		"\U00002595"	// left delimiter
		"\U0000258F"	// right delimiter
		"\U00002588"	// full block
		"\U0000258E"	// one-quarter block
		"\U0000258C"	// half block
		"\U0000258D"	// three-quarter block
		;
	static const int PROGRESS_DELIMITER_LEFT = 0;
	static const int PROGRESS_DELIMITER_RIGHT = 3;
	static const int PROGRESS_FULL_BLOCK = 6;
	static inline const int PROGRESS_PARTIAL_BLOCKS[4] = { 0, 9, 12, 15 };

	void setBlockChar(char* p, int code)
	{
		p[0] = BLOCK_CHARS_UTF8[code];
		p[1] = BLOCK_CHARS_UTF8[code+1];
		p[2] = BLOCK_CHARS_UTF8[code+2];
	}

	char display_[128];
	uint64_t totalUnits_;
	uint64_t unitsCompleted_;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
	std::chrono::time_point<std::chrono::steady_clock> lastReportTime_;
	int percentageCompleted_;
	int taskWidth_ = MAX_TASK_WIDTH;	// maximum number of characters for task display
};
