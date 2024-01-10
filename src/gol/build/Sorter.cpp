#include "Sorter.h"
#include <cassert>

/*
void SorterContext::encodeTags(protobuf::Message keys, protobuf::Message values)
{
    int count = countVarints(keys.p, keys.pEnd);

    assert(tagsOrRoles_.empty());
    const uint8_t* pKey = keys.p;
    const uint8_t* pValue = values.p;
    while (pKey < keys.pEnd)
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
*/

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
