// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <Python.h>
#include <string_view>

namespace PyHash
{
	/**
	 * CPython-compatible hash of an integer value: hash(int) is the value
	 * itself for anything that fits in 61 bits, except that a Python hash
	 * function must never return -1 (reserved for errors).
	 */
	inline Py_hash_t intHash(int64_t v)
	{
		return v == -1 ? -2 : static_cast<Py_hash_t>(v);
	}

	/**
	 * Mirrors CPython's tuple hash (xxHash-based, stable since CPython 3.8)
	 * for a 2-tuple, so hash(Coordinate(x,y)) == hash((x,y)). Required for
	 * dict/set correctness because Coordinate compares equal to 2-element
	 * sequences.
	 */
	inline Py_hash_t tupleHash2(Py_hash_t e1, Py_hash_t e2)
	{
		const uint64_t P1 = 11400714785074694791ULL;   // _PyHASH_XXPRIME_1
		const uint64_t P2 = 14029467366897019727ULL;   // _PyHASH_XXPRIME_2
		const uint64_t P5 = 2870177450012600261ULL;    // _PyHASH_XXPRIME_5
		uint64_t acc = P5;
		const uint64_t lanes[2] =
		{
			static_cast<uint64_t>(e1),
			static_cast<uint64_t>(e2)
		};
		for (uint64_t lane : lanes)
		{
			acc += lane * P2;
			acc = (acc << 31) | (acc >> 33);
			acc *= P1;
		}
		acc += 2 ^ (P5 ^ 3527539ULL);
		if (acc == static_cast<uint64_t>(-1)) return 1546275796;
		return static_cast<Py_hash_t>(acc);
	}

	/**
	 * splitmix64 finalizer: cheap full-avalanche mix. Use this to hash
	 * packed coordinate values; without it, sign-extended negative
	 * coordinates (western/southern hemisphere) smear 1-bits across the
	 * upper word and collapse hash diversity.
	 */
	inline uint64_t mix64(uint64_t h)
	{
		h ^= h >> 30; h *= 0xbf58476d1ce4e5b9ULL;
		h ^= h >> 27; h *= 0x94d049bb133111ebULL;
		h ^= h >> 31;
		return h;
	}

	inline Py_hash_t asPyHash(uint64_t h)
	{
		Py_hash_t r = static_cast<Py_hash_t>(h);
		return r == -1 ? -2 : r;
	}

	/** Packs two int32 coordinates into a uint64 without sign extension. */
	inline uint64_t packCoords(int32_t hi, int32_t lo)
	{
		return (static_cast<uint64_t>(static_cast<uint32_t>(hi)) << 32) |
			static_cast<uint32_t>(lo);
	}

}

