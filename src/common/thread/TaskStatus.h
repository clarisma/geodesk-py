// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
#include <string>


class TaskStatus
{
public:
	TaskStatus() : details_(nullptr) {}
	~TaskStatus()
	{
		set(nullptr);
	}

	enum
	{
		CODE_NORMAL_SHUTDOWN = 1,
		CODE_ABORTED = 2,
		CODE_BAD_ALLOC = 3,
		CODE_ERROR = 4
	};

	class Details
	{
	public:
		Details(int code) :
			code_(code) {}

		int code() const { return code_; }
		
	private:
		const int code_;
	};

	class ExtendedDetails : public Details
	{
	public:
		ExtendedDetails(int code, std::string msg) :
			Details(code),
			message_(msg)
		{
			assert(code >= CODE_ERROR);
		}

		std::string message() const { return message_; }

	private:
		std::string message_;
	};

	const Details* get() const
	{
		return details_.load();
	}

	void set(const Details* newDetails)
	{
		const Details* oldDetails = details_.exchange(newDetails);
		if (oldDetails && oldDetails->code() >= CODE_ERROR)
		{
			delete reinterpret_cast<const ExtendedDetails*>(oldDetails);
		}
	}



	static const Details NORMAL_SHUTDOWN;
	static const Details ABORTED;
	static const Details BAD_ALLOC;

private:
	std::atomic<const Details*> details_;
};
