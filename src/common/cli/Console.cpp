// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only
 
#include "Console.h"
#include <cstdlib>

template <size_t N>
static char* putString(char* p, const char(&s)[N])
{
	memcpy(p, s, N);
	return p + N;
}

Console::Console()
{
	hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD consoleMode;
	GetConsoleMode(hConsole_, &consoleMode);
	consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hConsole_, consoleMode);
	SetConsoleOutputCP(CP_UTF8);
}

void Console::start(int64_t totalWork, std::string_view task)
{
	totalWork_ = totalWork;
	workCompleted_ = 0;
	currentTask_ = task;
	startTime_ = std::chrono::steady_clock::now();
	nextReportTime_ = std::chrono::steady_clock::time_point::min();
	status_[8] = ' ';
	putString(&status_[9], "\x1b[35m");
	progress(0);
}

void Console::progress(int64_t work)
{
	workCompleted_ += work;
	int percentage = static_cast<int>(workCompleted_ * 100 / totalWork_);
	if (percentage != percentageCompleted_)
	{
		percentageCompleted_ = percentage;
		formatPercentage(&status_[14], percentage);
		status_[18] = ' ';
		char* p = drawProgressBar(&status_[19], percentage);
		p = putString(p, "\033[0m ");
		memset(p, ' ', MAX_TASK_CHARS);
		memcpy(p, currentTask_.data(), std::min(currentTask_.size(), 38ULL));
		p += MAX_TASK_CHARS;
		*p++ = '\r';
		statusLen_ = p - status_;
	}
	std::chrono::time_point<std::chrono::steady_clock> now(
		std::chrono::steady_clock::now());
	if (now >= nextReportTime_)
	{
		formatTimespan(status_, std::chrono::duration_cast
			<std::chrono::milliseconds>(now - startTime_));
		nextReportTime_ = std::chrono::ceil<std::chrono::seconds>(now);
		DWORD written;
		WriteConsoleA(hConsole_, status_, statusLen_, &written, NULL);
	}
}

void Console::formatTimespan(char* buf, std::chrono::milliseconds ms)
{
	div_t d;
	int s = ms.count() / 1000;
	d = div(s, 60);
	int m = d.quot;
	s = d.rem;
	d = div(m, 60);
	int h = d.quot;
	m = d.rem;
	d = div(h, 10);
	buf[0] = '0' + d.quot;
	buf[1] = '0' + d.rem;
	buf[2] = ':';
	d = div(m, 10);
	buf[3] = '0' + d.quot;
	buf[4] = '0' + d.rem;
	buf[5] = ':';
	d = div(s, 10);
	buf[6] = '0' + d.quot;
	buf[7] = '0' + d.rem;
}


void Console::formatPercentage(char* buf, int percentage)
{
	div_t d;
	int v1, v2, v3;
	d = div(percentage, 10);
	v3 = d.rem;
	d = div(d.quot, 10);
	v2 = d.rem;
	v1 = d.quot;
	buf[0] = v1 ? ('0' + v1) : ' ';
	buf[1] = (v1 | v2) ? ('0' + v2) : ' ';
	buf[2] = '0' + v3;
	buf[3] = '%';
}


void Console::setTask(std::string_view task)
{
	currentTask_ = task;
	nextReportTime_ = std::chrono::steady_clock::time_point::min();
	percentageCompleted_ = -1;
	progress(0);
}

const char* Console::BLOCK_CHARS_UTF8 =
	"\U00002588"	// full block
	"\U0000258E"	// one-quarter block
	"\U0000258C"	// half block
	"\U0000258A"	// three-quarter block
	;

char* Console::drawProgressBar(char* p, int percentage)
{
	p = putString(p, "\033[35;100m");
	int fullBlocks = percentage / 4;
	char* pEnd = p + fullBlocks * 3;
	while (p < pEnd)
	{
		*p++ = BLOCK_CHARS_UTF8[0];
		*p++ = BLOCK_CHARS_UTF8[1];
		*p++ = BLOCK_CHARS_UTF8[2];
	}
	int partialBlocks = fullBlocks % 4;
	int emptyBlocks;
	if (partialBlocks)
	{
		const char* pChar = &BLOCK_CHARS_UTF8[partialBlocks * 3];
		*pEnd++ = *pChar++;
		*pEnd++ = *pChar++;
		*pEnd++ = *pChar;
		emptyBlocks = 25 - fullBlocks - 1;
	}
	else
	{
		emptyBlocks = 25 - fullBlocks;
	}
	p = pEnd;
	pEnd = p + emptyBlocks;
	while (p < pEnd)
	{
		*p++ = ' ';
	}
	return p;
}

