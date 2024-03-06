// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only
 
#if defined(_WIN32) 
#include "Console_windows.cxx"
#elif defined(__linux__) || defined(__APPLE__) 
#include "Console_linux.cxx"
#else
#error "Platform not supported"
#endif

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

void Console::start(int64_t totalWork, std::string_view task)
{
	totalWork_ = totalWork;
	workCompleted_ = 0;
	currentTask_ = task;
	startTime_ = std::chrono::steady_clock::now();
	nextReportTime_ = std::chrono::steady_clock::time_point::min();
	status_[8] = ' ';
	putString(&status_[9], "\x1b[33m");
	progress(0);
}

// TODO: proper locking

void Console::log(std::string_view msg)
{
	char buf[1024];
	char* p = putString(buf, "\u001b[38;5;242m");
	std::chrono::time_point<std::chrono::steady_clock> now(
		std::chrono::steady_clock::now());
	formatTimespan(p, std::chrono::duration_cast
		<std::chrono::milliseconds>(now - startTime_), true);
	p = putString(p + 12, "\u001b[0m");
	size_t maxTextLen = consoleWidth_ - 15;
	memset(p, ' ', maxTextLen + 2);
	p += 2;
	size_t actualTextLen = std::min(msg.length(), maxTextLen);
	memcpy(p, msg.data(), actualTextLen);
	p += maxTextLen;
	*p++ = '\n';
	print(buf, p - buf);
	nextReportTime_ = std::chrono::steady_clock::time_point::min();
	progress(0);
}

// TODO: proper locking

void Console::progress(int64_t work)
{
	std::unique_lock<std::mutex> lock(mutex_);

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
		print(status_, statusLen_);
	}
}

void Console::formatTimespan(char* buf, std::chrono::milliseconds timeSpan, bool withMs)
{
	div_t d = div(static_cast<int>(timeSpan.count()), 1000);
	Format::timeFast(buf, d.quot, withMs ? d.rem : -1);
}


char* Console::formatPercentage(char* buf, int percentage)
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
	return &buf[4];
}


void Console::setTask(std::string_view task)
{
	currentTask_ = task;
	nextReportTime_ = std::chrono::steady_clock::time_point::min();
	percentageCompleted_ = -1;
	progress(0);
}

const char* Console::BLOCK_CHARS_UTF8 = (const char*)
	u8"\U00002588"	// full block
	u8"\U0000258E"	// one-quarter block
	u8"\U0000258C"	// half block
	u8"\U0000258A"	// three-quarter block
	;

char* Console::drawProgressBar(char* p, int percentage)
{
	p = putString(p, "\033[33;100m");
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
