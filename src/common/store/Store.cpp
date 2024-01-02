// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Store.h"
#include <common/util/BitIterator.h>
#include <common/util/log.h>
#include <common/util/pointer.h>
#include <cassert>
#include <filesystem>
#include <zlib.h>

Store::Store() :
    openMode_(0),
    lockLevel_(LOCK_NONE)
{
}


Store::LockLevel Store::lock(LockLevel newLevel)
{
    LockLevel oldLevel = lockLevel_;
    if (newLevel != oldLevel)
    {
        if (lockLevel_ == LOCK_EXCLUSIVE || newLevel == LOCK_NONE)
        {
            lockRead_.release();
            lockLevel_ = LOCK_NONE;
        }
        if (lockLevel_ == LOCK_NONE && newLevel != LOCK_NONE)
        {
            lockRead_.lock(handle(), 0, 4, newLevel != LOCK_EXCLUSIVE);
        }
        if (oldLevel == LOCK_APPEND)
        {
            lockWrite_.release();
        }
        if (newLevel == LOCK_APPEND)
        {
            lockWrite_.lock(handle(), 4, 4, false);
        }
        lockLevel_ = newLevel;
        // LOG("Lock level is now %d", newLevel);
    }
    return oldLevel;
}


bool Store::tryExclusiveLock()
{
    assert(lockLevel_ == LOCK_NONE);
    if (!lockRead_.tryLock(handle(), 0, 4, false)) return false;
    lockLevel_ = LOCK_EXCLUSIVE;
    return true;
}

void Store::open(const char* filename, int mode)
{
    if (isOpen()) throw StoreException("Store is already open");

    fileName_ = filename;

    // LOG("Opening %s (Mode %d) ...", filename, mode);

    ExpandableMappedFile::open(filename, (mode & ~OpenMode::EXCLUSIVE) |
        File::OpenMode::READ);
        // Don't pass EXCLUSIVE to base because it has no meaning

    // TODO: Ideally, we should lock before mapping (Creating a writable 
    // mapping can grow the file, if we can't obtain the lock we may get
    // an exception -- ideally, we should catch the exception and then
    // shrink back the file)
    lock((mode & OpenMode::EXCLUSIVE) ? LOCK_EXCLUSIVE : LOCK_READ);

    pointer p(mainMapping());

    // TODO: if mode is not CREATE, throw an exception instead
    if (p.getUnsignedInt() == 0) createStore();
        
    /*
    File journalFile = getJournalFile();
    if (journalFile.exists())
    {
        processJournal(journalFile);
    }
    */
      
    verifyHeader();     // TODO: when to do this?
    initialize();
    
    // TODO: turn IOException into StoreException?
}


void Store::close()
{
    if(!isOpen()) return;     // TODO: spec this behavior
    
    uint64_t trueSize = getTrueSize();
    bool journalPresent = false;
    /*
    File journalFile = getJournalFile();
    if (journal != null)
    {
        journalPresent = true;
    }
    if (!journalPresent)
    {
        journalPresent = journalFile.exists();
    }
    */
      
    lock(LOCK_NONE);
    bool segmentUnmapAttempted = false;

    if (journalPresent || trueSize > 0)
    {
        if (tryExclusiveLock())
        {
            if (journalPresent)
            {
                /*
                if (processJournal(journalFile))
                {
                    // Get true file size again, because it may have
                    // changed after journal instructions were processed

                    trueSize = getTrueSize();
                }

                if (journal != null)
                {
                    journal.close();
                    journal = null;
                }
                journalFile.delete();
                */
            }
            if (trueSize > 0)
            {
                segmentUnmapAttempted = true;
                unmapSegments();
                setSize(trueSize);
            }
            lock(LOCK_NONE);
        }
    }
    if (!segmentUnmapAttempted) unmapSegments();
    ExpandableMappedFile::close();
    fileName_.clear();
}


void Store::error(const char* msg) const
{
    throw StoreException(fileName(), msg);
}


void Store::checkJournal()
{
    std::string journalFileName = getJournalFileName();
    if (!std::filesystem::exists(journalFileName)) return;

    File journalFile;
    journalFile.open(journalFileName.c_str(), File::OpenMode::READ);
    uint32_t instruction;
    journalFile.read(&instruction, 4);
    if (instruction != 0)
    {
        // Even though we may be making modifications other than additions,
        // we only need to obtain the append lock: If another process died
        // while making additions, then exclusive read access isn't necessary.
        // If another process made modifications that did not complete
        // normally, it would have had to hold exclusive read access -- this
        // means that if we are here, we have been waiting to open the file,
        // so we are the first to see the journal instructions.

        LockLevel prevLockLevel = lock(LOCK_APPEND);  // TODO: need exclusive lock!

        // Check header again, because another process may have already
        // processed the journal while we were waiting for the lock

        journalFile.seek(0);
        journalFile.read(&instruction, 4);
        if (instruction != 0)
        {
            if (isJournalValid(journalFile))
            {
                applyJournal(journalFile);
            }
        }
        lock(prevLockLevel);
    }
    journalFile.close();
}

bool Store::isJournalValid(File& file)
{
    uint64_t journalSize = file.size();
    if (journalSize < 24 || (journalSize & 3) != 0) return false;

    // TODO: Here, we assume Little-Endian byte order, which differs from Java

    uint64_t timestamp;
    file.seek(4);
    file.read(&timestamp, 8);
    if (timestamp != getLocalCreationTimestamp()) return false;

    uint32_t crc = crc32(0L, Z_NULL, 0);  // Initialize the CRC
    uint32_t patchWordsRemaining = (journalSize - 24) / 4;
    while (patchWordsRemaining)
    {
        uint32_t patchWord;
        file.read(&patchWord, 4);
        crc32(crc, reinterpret_cast<const Bytef*>(&patchWord), 4);
        patchWordsRemaining--;
    }

    uint64_t endMarker;
    file.read(&endMarker, 8);
    if (endMarker != JOURNAL_END_MARKER) return false;

    uint32_t journalCrc;
    file.read(&journalCrc, 4);
    return journalCrc == crc;
}


void Store::applyJournal(File& file)
{
    uint64_t storeFileSize = size();

    file.seek(12);
    for (;;)
    {
        uint64_t patch;
        file.read(&patch, 8);
        if (patch == JOURNAL_END_MARKER) break;
        uint64_t pos = (patch >> 10) << 2;
        uint32_t len = ((patch & 0x3ff) + 1) * 4;
        // Log.debug("Patching %d words at %X", len, pos);
        if ((pos + len) > storeFileSize)
        {
            error("Cannot restore from journal, store modified outside transaction");
            return;
        }
        if (file.read(mainMapping() + pos, len) != len)
        {
            error("Failed to apply patch from journal");
            return;
        }
    }
    sync(mainMapping(), storeFileSize);
}


Store::Transaction::Transaction(Store* store) :
    store_(store),
    preCommitStoreSize_(store->getTrueSize())
{
}


Store::Transaction::~Transaction()
{
    for (const auto& pair : blocks_) 
    {
        delete pair.second;
    }
}

uint8_t* Store::Transaction::getBlock(uint64_t pos)
{
    if (pos >= preCommitStoreSize_) return store_->translate(pos);
    TransactionBlock* block;
    auto it = blocks_.find(pos);
    if (it == blocks_.end())
    {
        block = new TransactionBlock(store_->translate(pos));
        blocks_.emplace(pos, block);
    }
    else
    {
        block = it->second;
    }
    return block->current();
}


const uint8_t* Store::Transaction::getConstBlock(uint64_t pos)
{
    TransactionBlock* block;
    auto it = blocks_.find(pos);
    if (it != blocks_.end())
    {
        return it->second->current();
    }
    return store_->translate(pos);
}


void Store::Transaction::commit()
{
    // TODO

    // Save the rollback instructions and make sure the journal file
    // is safely written to disk
    saveJournal();

    // TODO: order matters! Possible race condition where index blocks
    //  are written before tile contents -- make sure to write blocks
    //  that are part of metadata *last*

    uint32_t dirtyMappings = 0;
    for (const auto& it : blocks_)
    {
        uint64_t ofs = it.first;
        TransactionBlock* block = it.second;
        memcpy(block->original(), block->current(), TransactionBlock::SIZE);
        dirtyMappings |= 1 << store_->mappingNumber(ofs);
    }

    // Blocks that are appended to the file during the transaction are
    // not journaled (they are simply truncated in case of a rollback);
    // nevertheless, we need to record their segments as well, so we
    // can force them to be written to disk

    uint64_t newStoreSize = store_->getTrueSize();
    if (newStoreSize > preCommitStoreSize_)
    {
        int start = store_->mappingNumber(preCommitStoreSize_);
        int end = store_->mappingNumber(newStoreSize-1);
        for (int n = start; n <= end; n++)
        {
            dirtyMappings |= 1 << n;
        }
    }

    // Ensure that the modified mappings are written to disk
    BitIterator<uint32_t> iter(dirtyMappings);
    for (;;)
    {
        int n = iter.next();
        if (n < 0) break;
        store_->sync(store_->mapping(n), store_->mappingSize(n));
    }
    
    clearJournal();

    preCommitStoreSize_ = newStoreSize;
}


void Store::Transaction::saveJournal()
{
    if (!journalFile_.isOpen())
    {
        journalFile_.open(store_->getJournalFileName().c_str(),
            File::OpenMode::READ | File::OpenMode::WRITE | File::OpenMode::CREATE);
    }
    journalFile_.seek(0);
    uint32_t command = 1;
    journalFile_.write(&command, 4);
    uint64_t timestamp = store_->getLocalCreationTimestamp();
    journalFile_.write(&timestamp, 8);
    uint32_t crc = crc32(0L, Z_NULL, 0);  // Initialize the CRC
    for (const auto& it: blocks_)
    {
        uint64_t baseWordAddress = it.first / 4;
        TransactionBlock* block = it.second;
        const uint32_t* original = reinterpret_cast<uint32_t*>(block->original());
        const uint32_t* current = reinterpret_cast<uint32_t*>(block->current());
        int n = 0;
        while(n < 1024)
        {
            if (original[n] != current[n])
            {
                int start = n;
                for (;;)
                {
                    n++;
                    if (n == 1024) break;
                    if (original[n] == current[n]) break;
                }
                int patchLen = n - start;
                uint64_t patch = ((baseWordAddress + start) << 10) | (patchLen - 1);
                journalFile_.write(&patch, 8);
                journalFile_.write(&original[start], patchLen * 4);
                crc32(crc, reinterpret_cast<const Bytef*>(&patch), 8);
                crc32(crc, reinterpret_cast<const Bytef*>(&original[start]), patchLen * 4);
            }
            n++;
        }
    }
    uint64_t trailer = JOURNAL_END_MARKER;
    journalFile_.write(&trailer, 8);
    journalFile_.write(&crc, 4);
    journalFile_.force();
}


void Store::Transaction::clearJournal()
{
    journalFile_.seek(0);
    uint32_t command = 0;
    journalFile_.write(&command, 4);
    journalFile_.setSize(4);   // TODO: just trim to 0 instead?
    journalFile_.force();
}
