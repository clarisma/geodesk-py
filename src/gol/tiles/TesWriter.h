#pragma once

#include <common/util/BufferWriter.h>
#include "TTile.h"

class TesWriter
{
public:
	TesWriter(TTile& tile, Buffer* out);

	void write();

private:
	void sortStrings();
	void sortTagTables();
	void sortRelationTables();
	void writeStrings();

	TString** strings_;
	TTagTable** tagTables_;
	TRelationTable** relationTables_;
	TFeature** features_;
	BufferWriter out_;
	TTile& tile_;
};
