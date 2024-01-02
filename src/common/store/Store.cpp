// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "Store.h"
#include <common/util/log.h>
#include <common/util/pointer.h>
#include <cassert>

Store::Store() :
    openMode_(0),
    lockLevel_(LOCK_NONE),
    mainMapping_(nullptr),
    mainMappingLength_(0)
{
    memset(extendedMappings_, 0, sizeof(extendedMappings_));
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
            lockRead_.lock(file_.handle(), 0, 4, newLevel != LOCK_EXCLUSIVE);
        }
        if (oldLevel == LOCK_APPEND)
        {
            lockWrite_.release();
        }
        if (newLevel == LOCK_APPEND)
        {
            lockWrite_.lock(file_.handle(), 4, 4, false);
        }
        lockLevel_ = newLevel;
        // LOG("Lock level is now %d", newLevel);
    }
    return oldLevel;
}


bool Store::tryExclusiveLock()
{
    assert(lockLevel_ == LOCK_NONE);
    if (!lockRead_.tryLock(file_.handle(), 0, 4, false)) return false;
    lockLevel_ = LOCK_EXCLUSIVE;
    return true;
}

void Store::open(const char* filename, int mode)
{
    if (file_.isOpen()) throw StoreException("Store is already open");

    fileName_ = filename;

    // LOG("Opening %s (Mode %d) ...", filename, mode);

    file_.open(filename,
        File::OpenMode::READ | File::OpenMode::SPARSE |
        ((mode & OpenMode::WRITE) ? (File::OpenMode::WRITE) : 0) |
        ((mode & OpenMode::CREATE) ? (File::OpenMode::CREATE | File::OpenMode::WRITE) : 0));
    lock((mode & OpenMode::EXCLUSIVE) ? LOCK_EXCLUSIVE : LOCK_READ);

    // Always do the following first, even if journal is present

    uint64_t fileSize = file_.size();
    // LOG("File size = %ld", fileSize);
    int mappingMode;
    if (mode & OpenMode::WRITE)
    {
        mainMappingLength_ = (fileSize + SEGMENT_LENGTH - 1) / SEGMENT_LENGTH * SEGMENT_LENGTH;
        file_.setSize(mainMappingLength_);
            // TODO: only needed on Linux, Windows expands the file automatically
            // to match the mapping extent
        mappingMode = MappedFile::MappingMode::READ | MappedFile::MappingMode::WRITE;
    }
    else
    {
        mainMappingLength_ = fileSize;
        mappingMode = MappedFile::MappingMode::READ;
    }
    // LOG("About to map main mapping (mode = %d) ...", mappingMode);
    mainMapping_ = file_.map(0, mainMappingLength_, mappingMode);
    // LOG("Mapped main mapping.", mappingMode);
    pointer p(mainMapping_);

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


void Store::unmapSegments()
{
    file_.unmap(mainMapping_, mainMappingLength_);
    // TODO: unmap extended segments
}


void Store::close()
{
    if(!file_.isOpen()) return;     // TODO: spec this behavior
    
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
                file_.setSize(trueSize);
            }
            lock(LOCK_NONE);
        }
    }
    if (!segmentUnmapAttempted) unmapSegments();
    file_.close();
    fileName_.clear();
}


void Store::error(const char* msg) const
{
    throw StoreException(fileName(), msg);
}
