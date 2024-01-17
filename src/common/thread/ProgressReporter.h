// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstdint>
#include <stdio.h>

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
		report();
	}

	void progress(uint64_t units)
	{
		unitsCompleted_ += units;
		int percentage = (int)(unitsCompleted_ * 100 / totalUnits_);
		if (percentage != percentageCompleted_)
		{
			percentageCompleted_ = percentage;
		}
	}

	void report()
	{
		printf("%s... %d%%\r", verb_, percentageCompleted_);
	}

private:
	const char* verb_;
	uint64_t totalUnits_;
	uint64_t unitsCompleted_;
	int percentageCompleted_;
};
