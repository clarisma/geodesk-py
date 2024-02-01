#include "Sorter.h"
#include <cassert>


SorterContext::SorterContext(Sorter* sorter) :
    OsmPbfContext<SorterContext, Sorter>(sorter)
{
}

SorterContext::~SorterContext()
{
}

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


void SorterContext::encodeTags(protobuf::Message keys, protobuf::Message values)
{
    const uint8_t* pKey = keys.start;
    const uint8_t* pValue = values.start;
    while (pKey < keys.end)
    {
        uint32_t key = readVarint32(pKey);
        uint32_t value = readVarint32(pValue);

        // TODO: Write encoded proto-string codes, or literal strings
        // into tempWriter_
    }

    // In new Proto-GOL format, we don't write the tag count
    // We just directly encode the tags into the temp buffer
    // The caller can then write the bodyLen and body contents
    // to the tile pile
}


void SorterContext::node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags)
{
    // project lon/lat to Mercator
    // look up tile
    // get tile encoder, or create new one (start group for nodes)
    // write x/y deltas to encoder
    // If tagged:
    // - write bodyLen (and tagCount?) to encoder
    // - write pre-encoded keys and values (or literals) to encoder
}

void SorterContext::way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes)
{
    // Need to track
    // - prevNodeTile
    // - northWestWayTile
    // - southEastWayTile
    // - encodedWayTile
    // - nodeTiles (if multi-tile)
    // - nodeCount (to determine if way has at least 2 nodes)
    //   - or, we can set encodedWayTile only within the loop
    //   - by default, it is zero
    //   - this tells us "< 2 nodes" as well as "all nodes missing";
    //     both cause way to be rejected
    // 
    // Go through all the nodes
    // Look up tile-pile of first node, set it as prev_node_tile
    // For each subsequent:
    // - look up the node's tile-pile
    // - If different from prev_node_tile:
    //   - way is multi-tile, use different approach:
    //     - For every node processed so far:
    //       - add prev_node_tile to node_tiles
    //     - determine common tile; this becomes tentative way tile
    // - If not multi-tile:
    //   - simply index prev_node_tile as way's tile  

    // Idea: we could use a tighter encoding for the multi-tile 
    // locator: Since we have 8 zoom levels max, we only need 8 values
    // for zoom delta, plus 2 bits that signal extra tile to east or south
    // we could encode these as 5 bits and put these into the id-delta
    // could even skip the multi-tile flag if we use 2 indicator bits:
    // 00 = not multi-tile
    // 01 = way lives in single tile (at higher zoom level)
    // 10 = way has eastern "buddy" tile (way at same or higher level)
    // 11 = way has southern "buddy" tile (way at same or higher level)

    // write node IDs (can copy directly from .osm.pbf, no re-encoding needed
    // write tags
}

void SorterContext::relation(int64_t id, protobuf::Message keys, protobuf::Message values,
	protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes)
{
}



Sorter::Sorter(int numberOfThreads) :
    OsmPbfReader(numberOfThreads),
    progress_("Sorting")
{

}
