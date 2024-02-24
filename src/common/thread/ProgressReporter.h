// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <chrono>
#include <cstdint>
#include <stdio.h>
#include <common/text/Format.h>

class ProgressReporter
{
public:
	ProgressReporter(const char* verb) :
		verb_(verb),
		totalUnits_(0),
		unitsCompleted_(0),
		percentageCompleted_(0)
	{
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
		printf("%s... %d%%\r", verb_, percentageCompleted_);
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
	const char* verb_;
	uint64_t totalUnits_;
	uint64_t unitsCompleted_;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
	std::chrono::time_point<std::chrono::steady_clock> lastReportTime_;
	int percentageCompleted_;
};
