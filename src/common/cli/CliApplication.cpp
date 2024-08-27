#include "CliApplication.h"

CliApplication* CliApplication::theApp_ = nullptr;

CliApplication::CliApplication()
{
	theApp_ = this;
}

void CliApplication::fail(std::string msg)
{
	out_.color(196);
	out_.writeString((const char*)u8" \u25A0\u25A0\u25A0\u25A0\u25A0\u25A0   ");
	out_.writeString(msg);
	out_.normal();
	out_.writeByte('\n');
	out_.flush();
}