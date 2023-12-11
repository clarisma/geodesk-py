// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <common/io/FileLock.h>
#include <common/io/ExpandableMappedFile.h>
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


class Store : protected ExpandableMappedFile
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

	inline uint8_t* data(uint64_t ofs) 
	{
		return translate(ofs);;
	}

	void error(const char* msg) const;

private:
	enum LockLevel
	{
		LOCK_NONE = 0,
		LOCK_READ = 1,
		LOCK_APPEND = 2,
		LOCK_EXCLUSIVE = 3
	};
	static const uint64_t SEGMENT_LENGTH = 1024 * 1024 * 1024;		// 1 GB
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
	
	static const uint64_t JOURNAL_END_MARKER = 0xffff'ffff'ffff'ffffUll;
	
	void checkJournal();
	std::string getJournalFileName() const
	{
		return fileName_ + ".journal";
	}
	bool isJournalValid(File& file);
	void applyJournal(File& file);

	std::string fileName_;
	int openMode_;
	LockLevel lockLevel_;
	FileLock lockRead_;
	FileLock lockWrite_;
	File journalFile_;
};

