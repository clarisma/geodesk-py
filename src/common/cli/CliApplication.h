// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Console.h"
#include "ConsoleWriter.h"

class CliApplication
{
public:
	CliApplication();

	void run(int argc, char* argv[]);

	ConsoleWriter& out() { return out_; }
	static CliApplication* get() { return theApp_; }

	void fail(std::string msg);

private:
	Console console_;
	ConsoleWriter out_;

	static CliApplication* theApp_;
};
