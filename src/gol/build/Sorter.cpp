#include "Sorter.h"
#include <cassert>


void SorterContext::stringTable(protobuf::Message strings)
{
    // Look up the ProtoStringEncoding for each string in the string table,
    // and store it in the String Translation Table. We have to do this for
    // each OSM block, since the same string (e.g. "highway") may have a
    // different code in each block. When we write the tags, we simply look
    // up the varint that represents the proto-string code for that string
    // (if the string is frequent enough in the entire .osm.pbf file to 
    // warrant inclusion in the Proto-String Table)
    // If the string is not in the Proto-String Table (because it occurs
    // infrequently), we store the offset of the string instead.

    osmStrings_ = strings.start;
    const uint8_t* p = strings.start;
    while(p < strings.end)
    {
        // TODO
        // Look up the string
        // If string is in the Proto-String Table, store the encodings
        // for key/value
        // Otherwise, store the offset
        uint32_t ofs = static_cast<uint32_t>(p + 1 - osmStrings_);
    }
}


int SorterContext::encodeTags(protobuf::Message keys, protobuf::Message values)
{
    const uint8_t* pKey = keys.start;
    const uint8_t* pValue = values.start;
    while (pKey < keys.end)
    {
        uint32_t key = readVarint32(pKey);
        uint32_t value = readVarint32(pValue);

    }
    while (tags.next())
    {
        tagsOrRoles.add(tags.key());
        tagsOrRoles.add(tags.stringValue());
    }
    body.writeVarint(tagsOrRoles.size() / 2);
    for (int i = 0; i < tagsOrRoles.size(); i += 2)
    {
        encodePackedString(tagsOrRoles.get(i), keyStrings);
        encodePackedString(tagsOrRoles.get(i + 1), valueStrings);
    }
    tagsOrRoles.clear();
}


void SorterContext::node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags)
{
}

void SorterContext::way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes)
{
}

void SorterContext::relation(int64_t id, protobuf::Message keys, protobuf::Message values,
	protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes)
{
}



Sorter::Sorter()
{

}
