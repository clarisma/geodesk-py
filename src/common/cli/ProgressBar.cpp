// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ProgressBar.h"


const char* ProgressBar::BLOCK_CHARS_UTF8 =
	"\U00002588"	// full block
	"\U0000258E"	// one-quarter block
	"\U0000258C"	// half block
	"\U0000258A"	// three-quarter block
	;

char* ProgressBar::draw(char* p, int percentage)
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
	p = putString(p, "\033[0m\r");
	*p = 0;
}
