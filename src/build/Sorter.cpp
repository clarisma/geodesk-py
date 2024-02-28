#include "Sorter.h"
#include <cassert>
#include "GolBuilder.h"
#include "geom/Mercator.h"

SorterContext::SorterContext(Sorter* sorter) :
    OsmPbfContext<SorterContext, Sorter>(sorter),
    builder_(sorter->builder()),
    osmStrings_(nullptr),
    nodeCount_(0),
    wayCount_(0),
    wayNodeCount_(0),
    relationCount_(0)
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
        uint32_t marker = readVarint32(p);
        if (marker != OsmPbf::STRINGTABLE_ENTRY)
        {
            throw OsmPbfException("Bad string table. Unexpected field: %d", marker);
        }
        const uint8_t* pString = p;
        uint32_t len = readVarint32(p);
        p += len;

        /* TODO
        stringTranslationTable_.push_back(
            builder_->stringCatalog().encodedProtoString(
                reinterpret_cast<const ShortVarString*>(pString),
                osmStrings_));
        */
    }
}


void SorterContext::encodeString(uint32_t stringNumber, int type)
{
    uint32_t code = stringTranslationTable_[stringNumber].get(type);
    if (code & ProtoStringCode::SHARED_STRING_FLAG)
    {
        // write the varint-encoded proto-string code (including the marker bit)
        uint32_t byteCount = (code & 3) + 1;
        code >>= 2;
        tempWriter_.writeBytes(reinterpret_cast<const uint8_t*>(&code), byteCount);
    }
    else
    {
        uint32_t ofs = code >> 3;
        const uint8_t* bytes = osmStrings_ + ofs;
        // TODO: wrong, must re-encode string length to account for the marker bit
        // maybe change encoding, use 0-byte as literal string marker
        // and use untagged varint for codes?
        // If so, would need to start numbering at 1
        // But how to diffentiate codes from offsets to literal?
        // could still use bit 2 as a marker
        uint32_t stringSize = reinterpret_cast<const ShortVarString*>(bytes)->totalSize();
        tempWriter_.writeBytes(bytes, stringSize);
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
    // TODO: clamp range
    Coordinate xy(Mercator::xFromLon100nd(lon100nd), Mercator::yFromLat100nd(lat100nd));
    uint32_t pile = builder_->tileCatalog().pileOfCoordinate(xy);
    builder_->nodeIndex().put(id, pile); // TODO: move to putput thread
    // look up tile
    // get tile encoder, or create new one (start group for nodes)
    // write x/y deltas to encoder
    // If tagged:
    // - write bodyLen (and tagCount?) to encoder
    // - write pre-encoded keys and values (or literals) to encoder
    nodeCount_++;
}

void SorterContext::way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes)
{
    IndexFile& nodeIndex = builder_->nodeIndex();
    const uint8_t* p = nodes.start;
    uint64_t nodeId = 0;
    uint32_t prevNodePile = 0;
    uint32_t nodeCount = 0;
    uint32_t wayPile = 0;
    while (p < nodes.end)
    {
        nodeId += readSignedVarint64(p);
        uint32_t nodePile = nodeIndex.get(nodeId);
        wayPile += nodePile;
        nodeCount++;
    }
    wayNodeCount_ += nodeCount;

    // TODO: this is a dummy
    builder_->wayIndex().put(id, wayPile / nodeCount);

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
    wayCount_++;
}

void SorterContext::relation(int64_t id, protobuf::Message keys, protobuf::Message values,
	protobuf::Message roles, protobuf::Message memberIds, protobuf::Message memberTypes)
{
    relationCount_++;
}


void SorterContext::harvestResults()
{
    reader()->addCounts(nodeCount_, wayCount_, wayNodeCount_, relationCount_);
}


Sorter::Sorter(GolBuilder* builder) :
    OsmPbfReader(builder->threadCount()),
    builder_(builder),
    progress_("Sorting"),
    nodeCount_(0),
    wayCount_(0),
    wayNodeCount_(0),
    relationCount_(0)
{

}

void Sorter::sort(const char* fileName)
{
    read(fileName, &progress_);
    char buf[200];
    Format::unsafe(buf, "Sorted %ld nodes / %ld ways (%ld way-nodes) / %ld relations",
        nodeCount_, wayCount_, wayNodeCount_, relationCount_);
    progress_.end(buf);
}
