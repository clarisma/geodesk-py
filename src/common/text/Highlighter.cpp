// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Highlighter.h"

void Highlighter::highlight(StringBuilder& buf, const char* text, 
	int ofs, int len, int color)
{
	// TODO: check if colors are supported in terminal
	bool colorize = false;
	// TOOD: find start line
	int start = 0;
	int excess = ofs + len - MAX_LINE_LEN;
	int padding = 0;
	int indent = 4;
	buf.writeRepeatedChar(' ', indent);
	if(excess > 0)
	{
		padding = 3;
		start += excess + padding;
		ofs -= excess + padding;
		buf.writeConstString("...");
	}
	buf.writeBytes(text + start, ofs);
	if (colorize)
	{
		buf.writeConstString("\033[");
		buf.formatInt(color);
		buf.writeByte('m');
	}
	buf.writeBytes(text + ofs, len);
	if (colorize) buf.writeConstString("\033[0m");
	buf.writeString(text + ofs + len);
	buf.writeByte('\n');
	buf.writeRepeatedChar(' ', indent + padding + ofs);
	if (colorize)
	{
		buf.writeConstString("\033[");
		buf.formatInt(color);
		buf.writeByte('m');
	}
	buf.writeRepeatedChar('^', len);
	if (colorize) buf.writeConstString("\033[0m");
}
