// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <common/io/ExpandableMappedFile.h>

/**
 * A fast disk-based key-value lookup that stores values in an array,
 * appropriate for keyspaces that are expected to be dense
 * 
 * - Values can be comprised of 1 to 24 bits and are stored in 
 *   packed form -- bits() must be called prior to use
 * - Care must be taken to use the same bit size for the same file
 *   (The bit size is not stored in the file)
 * - Values are stored within 4-KB blocks in a way that an access
 *   never reads across a block boundary (both for performance 
 *   reasons and to avoid segfaults)
 * - The file is sparse; blocks whose key range is unused don't
 *   take up disk space
 * - The file expands as needed, following the rules of the
 *   ExpandableMappedFile class
 */

class IndexFile : protected ExpandableMappedFile
{
public:
	IndexFile() : bits_(0) {}
	void bits(int bits)
	{
		assert(bits_ == 0);  // Can only be called once
		assert(bits >= 1 && bits <= 24);
		assert(!isOpen());
		bits_ = bits;
		slotsPerBlock_ = BLOCK_SIZE * 8 / bits;
		mask_ = (1 << bits) - 1;
	}

	uint32_t get(uint64_t key);
	void put(uint64_t key, uint32_t value);

private:
	static const uint32_t BLOCK_SIZE = 4096;

	int bits_;
	uint32_t slotsPerBlock_;
	uint32_t mask_;
};
