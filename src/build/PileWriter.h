// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstring>
#include <memory>
#include <common/alloc/SimpleArena.h>
#include <common/thread/Threads.h>
#include <common/util/BufferWriter.h>
#include <common/util/protobuf.h>
#include "geom/Coordinate.h"

class PileFile;


class PileSet
{
public:
	PileSet() :
		pageSize_(16 * 1024),
		arena_(16 * 1024 * 1024),	// must be multiple of 
		// Don't depend on pageSize_ (initialization order is based on 
		// order of fields, and might change)
		firstPile_(nullptr)
	{
	}

	PileSet(PileSet&& other) :
		arena_(std::move(other.arena_)),
		pageSize_(other.pageSize_),
		firstPile_(other.firstPile_)
	{
		other.firstPile_ = nullptr;
	}

	PileSet& operator=(PileSet&& other) noexcept 
	{
		if (this != &other) 
		{
			arena_ = std::move(other.arena_);
			pageSize_ = other.pageSize_;
			firstPile_ = other.firstPile_;
			other.firstPile_ = nullptr;
		}
		return *this;
	}


	void writeTo(PileFile& file);

	class Page
	{
	protected:
		Page* next_;

		friend class PileSet;
	};

	class Pile : public Page
	{
	public:
		Pile* next() const { return nextPile_; }

	private:
		Pile* nextPile_;
		uint32_t number_;
		uint32_t remaining_;
		uint8_t* p_;
		uint64_t prevId_;
		Coordinate prevCoord_;

		friend class PileSet;
		friend class PileWriter;
	};

protected:
	Pile* createPile(uint32_t number)
	{
		uint8_t* p = arena_.alloc(pageSize());
		Pile* pile = reinterpret_cast<Pile*>(p);
		pile->nextPile_ = firstPile_;
		pile->number_ = number;
		pile->remaining_ = pageSize() - sizeof(Pile);
		pile->p_ = p + sizeof(Pile);
		pile->prevId_ = 0;
		pile->prevCoord_ = Coordinate(0, 0);
		firstPile_ = pile;
		return pile;
	}

	void addPage(Pile* pile)
	{
		Page* lastPage = reinterpret_cast<Page*>(pile->p_ - pageSize() + pile->remaining_);
		uint8_t* p = arena_.alloc(pageSize());
		Page* nextPage = reinterpret_cast<Page*>(p);
		nextPage->next_ = nullptr;
		lastPage->next_ = nextPage;
		pile->p_ = p + sizeof(Page);
		pile->remaining_ = pageSize() - sizeof(Page);
	}

	uint32_t pageSize() const { return pageSize_; }

	SimpleArena arena_;
	uint32_t pageSize_;
	Pile* firstPile_;
};


class PileWriter : public PileSet
{
public:
	PileWriter(uint32_t tileCount)
	{
		Pile** pileIndex = new Pile*[tileCount + 1];
		memset(pileIndex, 0, sizeof(Pile*) * (tileCount + 1));
		pileIndex_.reset(pileIndex);
	}

	PileWriter(PileWriter&& other) :
		PileSet(std::move(other)),
		pileIndex_(std::move(other.pileIndex_))
	{
	}


	Pile* get(uint32_t pileNumber)
	{
		Pile* pile = pileIndex_.get()[pileNumber];
		if (!pile)
		{
			pile = createPile(pileNumber);
			pileIndex_.get()[pileNumber] = pile;
		}
		return pile;
	}
		
	void writeNode(uint32_t pileNumber, uint64_t id, Coordinate xy, BufferWriter& tags)
	{
		Pile* pile = get(pileNumber);
		uint8_t buf[32];
			// enough room for ID delta (with Bit 0 tagged),
			// x/y deltas, and optional tagsLength
		uint8_t* p = buf;
		bool hasTags = !tags.isEmpty();
		writeVarint(p, ((id - pile->prevId_) << 1) | static_cast<int>(hasTags));
		writeSignedVarint(p, xy.x - pile->prevCoord_.x);
		writeSignedVarint(p, xy.y - pile->prevCoord_.y);
		if (hasTags)
		{
			size_t tagsLen = tags.length();
			writeVarint(p, tagsLen);
			write(pile, buf, p - buf);
			write(pile, reinterpret_cast<const uint8_t*>(tags.data()), tagsLen);
		}
		else
		{
			write(pile, buf, p - buf);
		}
		pile->prevId_ = id;
		pile->prevCoord_ = xy;
	}

	void writeWay(uint32_t pileNumber, uint64_t id, protobuf::Message nodes, BufferWriter& tags)
	{
		Pile* pile = get(pileNumber);
		uint8_t buf[32];
			// enough room for ID delta (with Bit 0 tagged),
			// optional locator, and bodyLength
		bool isMultiTile = false; // TODO
		uint8_t* p = buf;
		writeVarint(p, ((id - pile->prevId_) << 1) | static_cast<int>(isMultiTile));
		write(pile, buf, p - buf);
		// TODO: locator
		size_t tagsLen = tags.length();
		size_t nodesLen = nodes.length();
		writeVarint(p, nodesLen + tagsLen);
		write(pile, buf, p - buf);
		write(pile, nodes.start, nodesLen);
		write(pile, reinterpret_cast<const uint8_t*>(tags.data()), tagsLen);
		pile->prevId_ = id;
	}

	void closePiles()
	{
		int xxx = 0;
		Pile* pile = firstPile_;
		while (pile)
		{
			writeByte(pile, 0);
			pileIndex_.get()[pile->number_] = nullptr;
			pile = pile->nextPile_;
			xxx++;
		}
		//printf("Closed %d piles.\n", xxx);
		printf("Thread %s: Flushing %d piles...\n", Threads::currentThreadId().c_str(), xxx);
	}

private:
	void write(Pile* pile, const uint8_t* bytes, uint32_t len)
	{
		for (;;)
		{
			if (len <= pile->remaining_)
			{
				std::memcpy(pile->p_, bytes, len);
				pile->p_ += len;
				pile->remaining_ -= len;
				return;
			}
			std::memcpy(pile->p_, bytes, pile->remaining_);
			bytes += pile->remaining_;
			len -= pile->remaining_;
			addPage(pile);
		}
	}

	void writeByte(Pile* pile, uint8_t v)
	{
		if (pile->remaining_ == 0) addPage(pile);
		*pile->p_ = v;
		pile->p_++;
		pile->remaining_--;
	}

	std::unique_ptr<Pile*[]> pileIndex_;
};
