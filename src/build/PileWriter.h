// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cstring>
#include <common/alloc/Arena.h>
#include <common/util/Buffer.h>
#include <common/util/protobuf.h>
#include "geom/Coordinate.h"

class PileWriter
{
public:
	PileWriter();

	class Page
	{
	protected:
		Page* next_;

		friend class PileWriter;
	};

	class Pile : public Page
	{
	public:
		

	private:
		Pile* nextPile_;
		uint32_t number_;
		uint32_t remaining_;
		uint8_t* p_;
		uint64_t prevId_;
		Coordinate prevCoord_;

		friend class PileWriter;
	};

	void writeNode(Pile* pile, uint64_t id, Coordinate xy, Buffer& tags)
	{
		uint8_t buf[32];
			// enough room for ID delta (with Bit 0 tagged),
			// x/y deltas, and optional tagsLength
		uint8_t* p = buf;
		bool hasTags = !tags.isEmpty();
		writeVarint(p, ((id - pile->prevId_) << 1) | hasTags);
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

	void writeWay(Pile* pile, uint64_t id, protobuf::Message nodes, Buffer& tags)
	{
		uint8_t buf[32];
			// enough room for ID delta (with Bit 0 tagged),
			// optional locator, and bodyLength
		bool isMultiTile = false; // TODO
		uint8_t* p = buf;
		writeVarint(p, ((id - pile->prevId_) << 1) | isMultiTile);
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

	void write(Pile* pile, const uint8_t* bytes, uint32_t len)
	{
		for (;;)
		{
			if (len < pile->remaining_)
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

private:
	Pile* createPile(uint32_t number, Pile* firstPile)
	{
		uint8_t* p = arena_.alloc(pageSize(), 8);
		Pile* pile = reinterpret_cast<Pile*>(p);
		pile->nextPile_ = firstPile;
		pile->number_ = number;
		pile->remaining_ = pageSize() - sizeof(Pile);
		pile->p_ = p + sizeof(Pile);
		pile->prevId_ = 0;
		pile->prevCoord_ = Coordinate(0,0);
		return pile;
	}

	void addPage(Pile* pile)
	{
		Page* lastPage = reinterpret_cast<Page*>(
			pile->p_ - pageSize() + pile->remaining_ - sizeof(Page));
		uint8_t* p = arena_.alloc(pageSize(), 8);
		Page* nextPage = reinterpret_cast<Page*>(p);
		nextPage->next_ = nullptr;
		lastPage->next_ = nextPage;
		pile->p_ = p + sizeof(Page);
		pile->remaining_ = pageSize() - sizeof(Page);
	}

	uint32_t pageSize() const { return pageSize_; }

	Arena arena_;
	uint32_t pageSize_;
};
