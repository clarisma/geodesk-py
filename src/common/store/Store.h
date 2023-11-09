// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <common/io/FileLock.h>
#include <common/io/MappedFile.h>
#include <common/util/enum_flags.h>

class StoreException : public IOException
{
public:
	explicit StoreException(const char* message)
		: IOException(message) {}

	explicit StoreException(const std::string& message)
		: IOException(message) {}

	explicit StoreException(const std::string& fileName, const char* message)
		: IOException(fileName + ": " + message) {}

	explicit StoreException(const std::string& fileName, const std::string& message)
		: IOException(fileName + ": " + message) {}
};


class Store
{
public:
	enum OpenMode
	{
		WRITE = 1 << 1,			// TODO: expected to match File::OpenMode
		CREATE = 1 << 2,		// TODO: expected to match File::OpenMode
								// TODO: Create always implies WRITE
		EXCLUSIVE = 1 << 7
	};

	Store();

	void open(const char* filename, int /* OpenMode */ mode);
	void close();

	const std::string& fileName() const { return fileName_; }

protected:
	virtual void createStore() = 0;
	virtual void verifyHeader() const = 0;
	virtual void initialize() = 0;
	virtual uint64_t getLocalCreationTimestamp() const = 0;
	virtual uint64_t getTrueSize() const = 0;
	// void* mapSegment(uint32_t segNumber, uint32_t segCount);

	inline uint8_t* data(uint64_t ofs) const
	{
		// TODO: extended mappings
		assert(ofs < mainMappingLength_);
		return reinterpret_cast<uint8_t*>(mainMapping_) + ofs;
	}

	void* mainMapping() const { return mainMapping_; }
	void error(const char* msg) const;
	void prefetch(const void* p, size_t len)
	{
		file_.prefetch(p, len);
	}

private:
	enum LockLevel
	{
		LOCK_NONE = 0,
		LOCK_READ = 1,
		LOCK_APPEND = 2,
		LOCK_EXCLUSIVE = 3
	};
	static const uint64_t SEGMENT_LENGTH = 1024 * 1024;		// 1 GB
	static const int EXTENDED_MAPPINGS_SLOT_COUNT = 16;

	class TransactionBlock
	{
	public:
		static const int SIZE = 4096;

	private:
		uint64_t pos_;
		void* original_;
		uint8_t current_[SIZE];
	};

	LockLevel lock(LockLevel newLevel);
	bool tryExclusiveLock();
	void unmapSegments();

	std::string fileName_;
	MappedFile file_;
	int openMode_;
	LockLevel lockLevel_;
	FileLock lockRead_;
	FileLock lockWrite_;
	void* mainMapping_;
	uint64_t mainMappingLength_;
	/**
	 * This table holds the mappings for segments that are added as the Store
	 * grows in size, and is only used if the Store is writable. 
	 * The first slot holds the first 1-GB segment that comes immediately 
	 * after mainMapping_, the second slot holds 2 1-GB segments (as a single 
	 * 2-GB mapping), then 4-GB etc. This way, 16 slots are enough to accommodate 
	 * growth of about 64 TB since the store has been opened (When a store is 
	 * closed and reopened, all these new segments will be covered by 
	 * mainMapping_; this implementation differs com.clarisma.common.store.Store, 
	 * since Java's MappedByteBuffer is limited to an int32_t range).
	 * TODO: access to this table must be guarded by a mutex to make
	 * Store threadsafe.
	 */
	void* extendedMappings_[EXTENDED_MAPPINGS_SLOT_COUNT];
	File journalFile_;

};

