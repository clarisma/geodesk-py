// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ConsoleWriter.h"
#include "Console.h"

ConsoleWriter::ConsoleWriter() :
	buf_(1024)
{
	setBuffer(&buf_);
}


void ConsoleWriter::color(int color)
{
	writeConstString("\033[38;5;");
	formatInt(color);
	writeByte('m');
}

void ConsoleWriter::normal()
{
	writeConstString("\033[0m");
}


void ConsoleWriter::flush()
{
	Console::get()->print(data(), length());
	clear();
}
