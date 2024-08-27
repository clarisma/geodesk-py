// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <mutex>

template <size_t N>
class Phaser
{
public:
	Phaser(int nThreads) 
	{
		for (int i = 0; i < N; i++) countdowns_[i] = nThreads;
	}

	void advancePhase (int currentPhase, int newPhase)
	{
        assert(newPhase > currentPhase);
        assert(newPhase <= N);
        std::unique_lock<std::mutex> lock(mutex_);
        for (int i = currentPhase; i < newPhase; i++)
        {
            assert(countdowns_[i] > 0);
            countdowns_[i]--;
            if (countdowns_[i] == 0)
            {
                phaseStarted_.notify_all();
            }
        }
        while (countdowns_[newPhase-1] > 0)
        {
            phaseStarted_.wait(lock);
        }
	}

private:
	std::mutex mutex_;
	std::condition_variable phaseStarted_;
	int countdowns_[N];
};
