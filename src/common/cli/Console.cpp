// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include <cassert>
#if defined(_WIN32) 
#include "Console_windows.cxx"
#elif defined(__linux__) || defined(__APPLE__) 
#include "Console_linux.cxx"
#else
#error "Platform not supported"
#endif
#include <common/thread/Threads.h>

// Always print status after logging a line
// always print if task changed
// If time is different, update timestamp
// If % changed, update % and scrollbar

Console* Console::theConsole_ = nullptr;

template <size_t N>
static char* putString(char* p, const char(&s)[N])
{
	memcpy(p, s, N-1);	// Subtract 1 to exclude null terminator
	return p + N-1;
}

Console::Console()
{
	init();
	theConsole_ = this;
}

void Console::start(const char* task)
{
	startTime_ = std::chrono::steady_clock::now();
	currentPercentage_.store(0, std::memory_order_release);
	setTask(task);
}

void Console::log(std::string_view msg)
{
	auto elapsed = std::chrono::steady_clock::now() - startTime_;
	char buf[1024];
	int ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	div_t d;
	d = div(ms, 1000);
	int s = d.quot;
	ms = d.rem;

	char* p = putString(buf, "\u001b[38;5;242m");
	p = Format::timeFast(p, s, ms);
	p = putString(p, "\u001b[0m");
	size_t maxTextLen = consoleWidth_ - 15;
	memset(p, ' ', maxTextLen + 2);
	p += 2;
	size_t actualTextLen = std::min(msg.length(), maxTextLen);
	memcpy(p, msg.data(), actualTextLen);
	p += maxTextLen;
	*p++ = '\n';
	int percentage = currentPercentage_.load(std::memory_order_acquire);
	const char* task = currentTask_.load(std::memory_order_acquire);
	size_t bytesPrinted = printWithStatus(buf, p, elapsed, percentage, task);
	assert(bytesPrinted < sizeof(buf));
}


char* Console::formatPercentage(char* buf, int percentage)
{
	buf = putString(buf, "\x1b[33m");
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
	return &buf[4];
}



const char* Console::BLOCK_CHARS_UTF8 = (const char*)
	u8"\U00002588"	// full block
	u8"\U0000258E"	// one-quarter block
	u8"\U0000258C"	// half block
	u8"\U0000258A"	// three-quarter block
	;


char* Console::formatProgressBar(char* p, int percentage)
{
	p = putString(p, " \033[33;100m");
	int fullBlocks = percentage / 4;
	char* pEnd = p + fullBlocks * 3;
	while (p < pEnd)
	{
		*p++ = BLOCK_CHARS_UTF8[0];
		*p++ = BLOCK_CHARS_UTF8[1];
		*p++ = BLOCK_CHARS_UTF8[2];
	}
	int partialBlocks = percentage % 4;
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


char* Console::formatStatus(char* buf, int secs, int percentage, const char* task)
{
	char* p = Format::timeFast(buf, secs, -1);
	if (percentage >= 0)
	{
		*p++ = ' ';
		p = formatPercentage(p, percentage);
		p = formatProgressBar(p, percentage);
		p = putString(p, "\033[0m ");
		if (task)
		{
			char* pEnd = p + MAX_TASK_CHARS;
			while (*task && p < pEnd)
			{
				*p++ = *task++;
			}
			while (p < pEnd) *p++ = ' ';
		}
	}
	*p++ = '\r';
	return p;
}


void Console::setProgress(int percentage)
{
	auto elapsed = std::chrono::steady_clock::now() - startTime_;
	// std::chrono::steady_clock::duration reportNext;
	int oldPercentage = currentPercentage_.load(std::memory_order_relaxed);
	if (percentage != oldPercentage)
	{
		currentPercentage_.store(percentage, std::memory_order_release);
	}
	else
	{
		auto reportNext = reportNext_.load(std::memory_order_relaxed);
		if (elapsed < reportNext) return;
		percentage = -1;
	}
	char buf[256];
	size_t bytesPrinted = printWithStatus(buf, buf, elapsed, percentage);
	assert(bytesPrinted < sizeof(buf));
}


size_t Console::printWithStatus(char* buf, char* p,
	std::chrono::steady_clock::duration elapsed,
	int percentage, const char* task)
{
	reportNext_.store(std::chrono::ceil<std::chrono::seconds>(elapsed),
		std::memory_order_relaxed);
	int secs = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
	const char* end = formatStatus(p, secs, percentage, task);
	size_t size = end - buf;
	print(buf, size);
	return size;
}


void Console::setTask(const char* task)
{
	currentTask_.store(task, std::memory_order_release);
	char buf[256];
	int percentage = currentPercentage_.load(std::memory_order_acquire);
	printWithStatus(buf, buf, std::chrono::steady_clock::now() - startTime_,
		currentPercentage_, task);
}


void Console::debug(const char* format, ...)
{
	if (theConsole_)
	{
		char buf[1024];
		char* p = buf;
		*p++ = '[';
		std::string threadId = Threads::currentThreadId();
		memcpy(p, threadId.data(), threadId.length());
		p += threadId.length();
		*p++ = ']';
		*p++ = ' ';
		va_list args;
		va_start(args, format);
		Format::unsafe(p, format, args);
		va_end(args);
		theConsole_->log(std::string_view(buf));
	}
}
