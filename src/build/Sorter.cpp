#include "Sorter.h"
#include <cassert>
#include "GolBuilder.h"
#include "geom/Mercator.h"

// TODO: Remember to always flush the BufferWriter, and reset buffer

SorterContext::SorterContext(Sorter* sorter) :
    OsmPbfContext<SorterContext, Sorter>(sorter),
    builder_(sorter->builder()),
    osmStrings_(nullptr),
    tempBuffer_(4096),
    tempWriter_(&tempBuffer_),
    currentPhase_(0),
    nodeCount_(0),
    wayCount_(0),
    wayNodeCount_(0),
    relationCount_(0),
    pileWriter_(sorter->builder()->tileCatalog().tileCount()),
    batchCount_(0)
{
//    features_.reserve(batchSize(0));
}

SorterContext::~SorterContext()
{
}

void SorterContext::stringTable(protobuf::Message strings)
{
    assert(stringTranslationTable_.empty());

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
        const ShortVarString* str = reinterpret_cast<const ShortVarString*>(p);
        stringTranslationTable_.push_back(
            builder_->stringCatalog().encodedProtoString(str, osmStrings_));
        p += str->totalSize();
    }
}


void SorterContext::encodeString(uint32_t stringNumber, int type)
{
    assert(stringNumber < stringTranslationTable_.size());  // TODO: exception?
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
        uint32_t len = *bytes;
        if (len & 0x80)
        {
            bytes++;
            len = (len & 0x7f) | (*bytes << 7);
        }
        uint32_t encodedLen = len << 1;
        if (encodedLen > 0x7f)
        {
            tempWriter_.writeByte((encodedLen & 0x7f) | 0x80);
            tempWriter_.writeByte(encodedLen >> 7);
            // TODO: We are encoding the string length as a varint13, since
            // we're using Bit 0 as the shared-vs-literal disciminator 
            // This limits string length to ~8K instead of ~16K
        }
        else
        {
            tempWriter_.writeByte(encodedLen);
        }
        tempWriter_.writeBytes(bytes+1, len);
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
        encodeString(key, ProtoStringCode::KEY);
        encodeString(value, ProtoStringCode::VALUE);
    }

    // In new Proto-GOL format, we don't write the tag count
    // We just directly encode the tags into the temp buffer
    // The caller can then write the bodyLen and body contents
    // to the tile pile
}


void SorterContext::encodeTags(protobuf::Message tags)
{
    const uint8_t* p = tags.start;
    while (p < tags.end)
    {
        uint32_t key = readVarint32(p);
        if (key == 0) break;
        uint32_t value = readVarint32(p);
        encodeString(key, ProtoStringCode::KEY);
        encodeString(value, ProtoStringCode::VALUE);
    }
    tags.start = p;
        // TODO: This feels hacky; start/end should be immutable, and node()
        // should not have the responsibility to advance start pointer.
        // However, this is the fastest approach
}


void SorterContext::addFeature(uint64_t id, uint32_t pile)
{
    // features_.emplace_back(id, pile);
    //if (features_.size() == features_.capacity()) flush(currentPhase_);

    IndexFile& index = builder_->featureIndex(currentPhase_);
    index.put(id, pile);  // TODO: needs to be synchronized
    batchCount_++;
    if (batchCount_ >= batchSize(currentPhase_)) flush(currentPhase_);
}

// TODO: Could deadlock if all tasks have been processed, but 
//  ways are still remaining unflushed while other threads are
//  attempting to start relations
//  afterTasks() should solve this


void SorterContext::afterTasks()
{
    if (blockBytesProcessed())
    {
        flush(Sorter::Phase::SUPER_RELATIONS);
    }
    else
    {
        reader()->advancePhase(currentPhase_, Sorter::Phase::SUPER_RELATIONS);
    }
}

void SorterContext::flush(int futurePhase)
{
    pileWriter_.closePiles();
    SorterOutputTask task(currentPhase_, futurePhase, blockBytesProcessed(),
        /* std::move(features_), */ std::move(pileWriter_));
    reader()->postOutput(std::move(task));
    resetBlockBytesProcessed();
    // features_.reserve(batchSize(futurePhase));
    batchCount_ = 0;
    // printf("Thread %s: Flushed.\n", Threads::currentThreadId().c_str());
    if (futurePhase != currentPhase_)
    {
        reader()->advancePhase(currentPhase_, futurePhase);
        currentPhase_ = futurePhase;
        /*
        if (futurePhase == 1)
        {
            printf("Started ways...\n");
        }
        if (futurePhase == 2)
        {
            printf("Started relations...\n");
        }
        */
    }
}

void SorterContext::node(int64_t id, int32_t lon100nd, int32_t lat100nd, protobuf::Message& tags)
{
    assert(tempWriter_.isEmpty());
    assert(id < 1'000'000'000'000ULL);
    // project lon/lat to Mercator
    // TODO: clamp range
    Coordinate xy(Mercator::xFromLon100nd(lon100nd), Mercator::yFromLat100nd(lat100nd));
    uint32_t pile = builder_->tileCatalog().pileOfCoordinate(xy);
    if (!pile)
    {
        Console::msg("node/%lld: Unable to assign to tile\n", id);
    }
    encodeTags(tags);
    pileWriter_.writeNode(pile, id, xy, tempWriter_);
    tempWriter_.clear();
    addFeature(id, pile);
    nodeCount_++;
}

/*
void SorterContext::writeWay(uint32_t pile, uint64_t id)
{
    pileWriter_.writeWay
}
*/

void SorterContext::way(int64_t id, protobuf::Message keys, protobuf::Message values, protobuf::Message nodes)
{
    assert(tempWriter_.isEmpty());
    if (currentPhase_ != Sorter::Phase::WAYS)
    {
        // TODO: phase shift
        flush(Sorter::Phase::WAYS);
    }

    encodeTags(keys, values);

    IndexFile& nodeIndex = builder_->nodeIndex();
    uint64_t nodeId = 0;
    uint32_t prevNodePile = 0;
    uint32_t nodeCount = 0;
    uint32_t wayPile = 0;
    
    const uint8_t* p = nodes.start;
    while (p < nodes.end)
    {
        nodeId += readSignedVarint64(p);
        uint32_t nodePile = nodeIndex.get(nodeId);
        if (!nodePile)
        {
            // printf("node/%llu not found in node index\n", nodeId);
        }
        if (nodePile != prevNodePile && prevNodePile)
        {
            // TODO: multi-tile way
        }

        // TODO: Error if node not in index

        prevNodePile = nodePile;
        nodeCount++;
    }
    wayNodeCount_ += nodeCount;

    // TODO: Reject ways with less than 2 nodes

    wayPile = prevNodePile;
    if (wayPile)        // TODO
    {
        pileWriter_.writeWay(wayPile, id, nodes, nodeCount, tempWriter_);
        addFeature(id, wayPile);
        //printf("way/%lld sorted into pile #%d\n", id, wayPile);
    }
    else
    {
        //printf("way/%lld: unable to sort\n", id);
    }
    tempWriter_.clear();

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
    assert(tempWriter_.isEmpty());
    if (currentPhase_ != Sorter::Phase::RELATIONS)
    {
        // TODO: phase shift
        flush(Sorter::Phase::RELATIONS);
    }

    uint64_t memberId = 0;
    uint32_t prevMemberPile = 0;
    uint32_t memberCount = 0;
    uint32_t relPile = 0;

    const uint8_t* pMemberId = memberIds.start;
    const uint8_t* pMemberType = memberTypes.start;
    const uint8_t* pRole = roles.start;
    while (pMemberId < memberIds.end)
    {
        memberId += readSignedVarint64(pMemberId);
        int memberType = *pMemberType++;
        uint32_t role = readVarint32(pRole);

        // TODO: remmeber the multi-tile bits in way/relation!!!
        uint32_t memberPile = builder_->featureIndex(memberType).get(memberId);

        if (!memberPile)
        {
            // printf("node/%llu not found in node index\n", nodeId);
        }
        if (memberPile != prevMemberPile && prevMemberPile != 0)
        {
            // TODO: multi-tile relation
        }

        // TODO: Member not in index

        tempWriter_.writeVarint((memberId << 2) | memberType);
        encodeString(role, ProtoStringCode::VALUE);

        prevMemberPile = memberPile;
        memberCount++;
    }
    
    encodeTags(keys, values);

    // TODO: Reject ways with less than 2 nodes

    relPile = prevMemberPile;  // TODO: dummy
    if (relPile)        // TODO
    {
        // TODO
        relPile = std::min(relPile, builder_->tileCatalog().tileCount() - 1);
        // assert(relPile < builder_->tileCatalog().tileCount());
        pileWriter_.writeRelation(relPile, id, memberCount, tempWriter_);
        addFeature(id, relPile);
        //printf("way/%lld sorted into pile #%d\n", id, wayPile);
    }
    else
    {
        //printf("way/%lld: unable to sort\n", id);
    }
    tempWriter_.clear();
    relationCount_++;
}

void SorterContext::endBlock()	// CRTP override
{
    stringTranslationTable_.clear();
}

void SorterContext::harvestResults()
{
    reader()->addCounts(nodeCount_, wayCount_, wayNodeCount_, relationCount_);
}


Sorter::Sorter(GolBuilder* builder) :
    OsmPbfReader(builder->threadCount()),
    builder_(builder),
    nodeCount_(0),
    wayCount_(0),
    wayNodeCount_(0),
    relationCount_(0)
{
    for (int i = 0; i < 3; i++)
    {
        phaseCountdowns_[i] = builder->threadCount();
    }
}


void Sorter::processTask(SorterOutputTask& task)
{
    task.piles_.writeTo(builder_->featurePiles());
    /*
    IndexFile& index = builder_->featureIndex(task.currentPhase_);
    printf("Writing %llu features into the index...\n", task.features_.size());
    for (const FeatureIndexEntry entry : task.features_)
    {
        index.put(entry.id(), entry.pile());
    }
    */
    builder_->progress(task.bytesProcessed_ * workPerByte_);
    reportOutputQueueSpace();
    // printf("-> Written\n");
}

static const char* PHASE_TASK_NAMES[] =
{
    "Sorting nodes...",
    "Sorting ways...",
    "Sorting relations...",
    "Sorting relations..."      // Super-relations
};

void Sorter::advancePhase(int currentPhase, int newPhase)
{
    //Console::debug("Advancing phase from %d to %d...", currentPhase, newPhase);
    assert(newPhase > currentPhase);
    assert(newPhase <= 3);
    std::unique_lock<std::mutex> lock(phaseMutex_);
    for (int i = currentPhase; i < newPhase; i++)
    {
        assert(phaseCountdowns_[i] > 0);
        phaseCountdowns_[i]--;
        //Console::debug("Completed phase %d, countdown is now %d", i, phaseCountdowns_[i]);
        if (phaseCountdowns_[i] == 0)
        {
            builder_->console().setTask(PHASE_TASK_NAMES[newPhase]);
            phaseStarted_.notify_all();
        }
    }
    while (phaseCountdowns_[newPhase - 1] > 0)
    {
        //Console::debug("Waiting to proceed to phase %d...", newPhase);
        phaseStarted_.wait(lock);
    }
}

void Sorter::startFile(uint64_t size)		// CRTP override
{
    workPerByte_ = builder_->phaseWork(GolBuilder::Phase::SORT) / size;
    builder_->console().setTask(PHASE_TASK_NAMES[Phase::NODES]);
}


void Sorter::sort(const char* fileName)
{
    read(fileName);
    char buf[200];
    Format::unsafe(buf, "Sorted %ld nodes / %ld ways (%ld way-nodes) / %ld relations",
        nodeCount_, wayCount_, wayNodeCount_, relationCount_);
    // progress_.end(buf);
}
