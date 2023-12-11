// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Store.h"
#include <common/util/log.h>
#include <common/util/pointer.h>
#include <cassert>
#include <filesystem>

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

    ExpandableMappedFile::open(filename, mode & ~OpenMode::EXCLUSIVE);
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

    uint32_t patchWordsRemaining = (journalSize - 24) / 4;
    uint32_t patchWord;
    while (patchWordsRemaining)
    {

    }
    byte[] ba = new byte[4];
    CRC32 crc = new CRC32();
    try
    {
        journal.seek(4);
        long timestamp = journal.readLong();
        if (timestamp != getTimestamp()) return false;
        for (; ; )
        {
            int patchLow = journal.readInt();
            int patchHigh = journal.readInt();
            if (patchHigh == 0xffff_ffff && patchLow == 0xffff_ffff) break;
            int len = (patchLow & 0x3ff) + 1;
            intToBytes(ba, patchLow);
            crc.update(ba);
            intToBytes(ba, patchHigh);
            crc.update(ba);
            for (int i = 0; i < len; i++)
            {
                intToBytes(ba, journal.readInt());
                crc.update(ba);
            }
        }
        return journal.readInt() == (int)crc.getValue();
    }
    catch (EOFException ex)
    {
        return false;
    }
}
