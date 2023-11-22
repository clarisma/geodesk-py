#include "TesWriter.h"

TesWriter::TesWriter(TTile& tile, Buffer* out) :
	tile_(tile),
	out_(out)
{
}


void TesWriter::write()
{

}


void TesWriter::writeStrings()
{
	size_t count = tile_.strings().count();
	out_.writeVarint(count);
	const TString** pEnd = strings_ + count;
	TString** p = strings_;
	while (p < pEnd)
	{
		out_.writeBytes((*p)->data(), (*p)->size());
	}
}


void TesWriter::sortStrings()
{
	size_t stringCount = tile_.strings().count();
	strings_ = tile_.strings().toArray(tile_.arena());

	// TODO: sort

	for (int i = 0; i < stringCount; i++)
	{
		strings_[i]->setLocation(i + 1);
	}
}


void TesWriter::sortTagTables()
{
	size_t tagTableCount = tile_.tagTables().count();
	tagTables_ = tile_.tagTables().toArray(tile_.arena());

	// TODO: sort

	for (int i = 0; i < tagTableCount; i++)
	{
		tagTables_[i]->setLocation(i + 1);
	}
}
