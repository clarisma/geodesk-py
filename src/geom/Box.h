// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <cassert>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include "geom/Coordinate.h"
#include <geos/geom/Envelope.h>

/// @brief An axis-aligned bounding box. A Box represents minimum and
/// maximum X and Y coordinates in a Mercator-projected plane. It can
/// straddle the Antimeridian (in which case minX is *larger* than maxX).
/// A Box can also be empty (in which case minY is *larger* than maxY).
///
/// A Box is considered *simple* if it is non-empty and does not straddle
/// the Antimeridian (i.e. `maxX >= minX && maxY >= minY`).
/// Box methods with `Simple` in their name assume that a Box is *simple*,
/// allowing a more efficient implementation (but return an invalid
/// result in case the Box is not).
///
class Box
{
public:
	constexpr Box() :
		m_minX(std::numeric_limits<int32_t>::max()), 
		m_minY(std::numeric_limits<int32_t>::max()),
		m_maxX(std::numeric_limits<int32_t>::min()),
		m_maxY(std::numeric_limits<int32_t>::min()) {}

	constexpr Box(int minX, int minY, int maxX, int maxY)
		: m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY) {}

	constexpr Box(Coordinate c)
		: m_minX(c.x), m_minY(c.y), m_maxX(c.x), m_maxY(c.y) {}

	// TODO: should this use floor/ceil to maximize bounds?
	Box(const geos::geom::Envelope* envelope) :
		m_minX(static_cast<int32_t>(std::round(envelope->getMinX()))),
		m_minY(static_cast<int32_t>(std::round(envelope->getMinY()))),
		m_maxX(static_cast<int32_t>(std::round(envelope->getMaxX()))),
		m_maxY(static_cast<int32_t>(std::round(envelope->getMaxY())))
	{
	}

	/// @brief Returns a Box that encompasses the entire world.
	///
	static constexpr Box ofWorld()
	{
		int constexpr min = std::numeric_limits<int32_t>::min();
		int constexpr max = std::numeric_limits<int32_t>::max();
		return Box(min, min, max, max);
	}

	/**
	 * Creates a simple Box regardless of order of coordinates.
	 */
	static Box normalizedSimple(Coordinate a, Coordinate b)
	{
		return Box(
			std::min(a.x, b.x), std::min(a.y, b.y),
			std::max(a.x, b.x), std::max(a.y, b.y));
	}

	static Box unitsAroundXY(int32_t d, Coordinate pt)
	{
		return Box(pt.x - d, trimmedSubtract(pt.y, d), pt.x + d, trimmedAdd(pt.y, d));
	}

	/// @brief A Box is *simple* if it is non-empty and does not straddle the Antimeridian.
	///
	bool isSimple() const
	{
		return m_maxX >= m_minX && m_maxY >= m_minY;
	}

	// TODO: this is simple
	bool intersects(const Box& other) const
	{
		return !(m_minX > other.m_maxX ||
			m_minY > other.m_maxY ||
			m_maxX < other.m_minX ||
			m_maxY < other.m_minY);
	}

	bool contains(int32_t x, int32_t y) const
	{
		if (minX() > maxX())
		{
			// empty or Antimeridian-crossing
			if (minY() > maxY()) return false;  // empty box cannot contain anything
			// If 180 longitude crossed, minX and maxY are swapped
			return (x >= maxX() && x <= minX() &&
				y >= minY() && y <= maxY());
		}
		return containsSimple(x, y);
	}

	bool containsSimple(int32_t x, int32_t y) const
	{
		return (!(x > m_maxX || y > m_maxY || x < m_minX || y < m_minY));
	}

	inline bool containsSimple(Coordinate c) const
	{
		return containsSimple(c.x, c.y);
	}

	inline bool contains(Coordinate c) const
	{
		return contains(c.x, c.y);
	}

	/**
	 * Assumes neither box is empty, and neither crosses Antimeridian.
	 */
	inline bool containsSimple(const Box& other) const
	{
		return other.minX() >= m_minX &&
			other.maxX() <= m_maxX &&
			other.minY() >= m_minY &&
			other.maxY() <= m_maxY;
	}

	bool contains(const Box& other) const
	{
		bool isSimple = minX() <= maxX();
		bool isOtherSimple = other.minX() <= other.maxX();
		if (isSimple && isOtherSimple) return containsSimple(other);

		// Either of the boxes is empty --> cannot contain or be contained
		if (minY() < maxY() || other.minY() < other.maxY()) return false;
		
		return std::min(other.minX(), other.maxX()) >= std::min(minX(), maxX()) &&
			std::max(other.minX(), other.maxX()) <= std::max(minX(), maxX()) &&
			other.minY() >= minY() && other.maxY() <= maxY();
	}


	/**
	 * Returns a Box that represents the intersection of two Box objects.
	 * This method assumes both are non-empty.
	 * Returns an empty Box if the boxes don't intersect.
	 */
	inline static Box simpleIntersection(const Box& a, const Box& b) 
	{
		assert(a.isSimple());
		assert(b.isSimple());
		
		int x1 = std::max(a.minX(), b.minX());
		int y1 = std::max(a.minY(), b.minY());
		int x2 = std::min(a.maxX(), b.maxX());
		int y2 = std::min(a.maxY(), b.maxY());

		if (x2 < x1 || y2 < y1) return Box();  // no intersection
		return Box(x1, y1, x2, y2);
	}

	int64_t widthSimple() const
	{
		return static_cast<int64_t>(maxX()) -
			static_cast<int64_t>(minX()) + 1;
	}

	int64_t height() const
	{
		return static_cast<int64_t>(maxY()) -
			static_cast<int64_t>(minY()) + 1;
	}


	inline double area() const
	{
		double w = (double)maxX() - (double)minX();
		double h = (double)maxY() - (double)minY();
		return w * h;
	}

	inline static const Box& simpleSmaller(const Box& a, const Box& b)
	{
		assert(a.isSimple());
		assert(b.isSimple());
		return (a.area() < b.area()) ? a : b;
	}

	// TODO: define and test Antimeridian behaviour
	inline void buffer(int32_t b)
	{
		if (isEmpty()) return;
		m_minX -= b;
		m_maxX += b;
		if (b >= 0)
		{
			m_minY = trimmedSubtract(m_minY, b);
			m_maxY = trimmedAdd(m_maxY, b);
		}
		else
		{
			m_minY = trimmedAdd(m_minY, -b);
			m_maxY = trimmedSubtract(m_maxY, -b);
			if (m_maxY < m_minY) setEmpty();
			// TODO: check if width flipped
		}
	}

	Coordinate center() const
	{
		return Coordinate(
			static_cast<int32_t>((static_cast<int64_t>(minX()) + maxX()) / 2),
			static_cast<int32_t>((static_cast<int64_t>(minY()) + maxY()) / 2));
	}

	int minX() const { return m_minX; } 
	int minY() const { return m_minY; }
	int maxX() const { return m_maxX; }
	int maxY() const { return m_maxY; }
	Coordinate topLeft() const { return Coordinate(minX(), maxY()); }
	Coordinate bottomRight() const { return Coordinate(maxX(), minY()); }
	Coordinate topRight() const { return Coordinate(maxX(), maxY()); }
	Coordinate bottomLeft() const { return Coordinate(minX(), minY()); }

	void setMinX(int32_t v) { m_minX = v;  }
	void setMinY(int32_t v) { m_minY = v; }
	void setMaxX(int32_t v) { m_maxX = v; }
	void setMaxY(int32_t v) { m_maxY = v; }

	bool isEmpty() const { return m_minY > m_maxY;  }

	/*
	int32_t operator[](const int index) const
	{
		assert(index >= 0 && index < 4);
		return (&m_minX)[index];
	}
	*/

	bool operator==(const Box& other) const
	{
		return memcmp(this, &other, sizeof(Box)) == 0;
	}

	bool operator!=(const Box& other) const
	{
		return memcmp(this, &other, sizeof(Box)) != 0;
	}

	int32_t& operator[](size_t index)
	{
		return (&m_minX)[index];
	}

	const int32_t& operator[](size_t index) const
	{
		return (&m_minX)[index];
	}

	void expandToIncludeSimple(const Box& b)
	{
		int otherMinX = b.minX();
		int otherMinY = b.minY();
		int otherMaxX = b.maxX();
		int otherMaxY = b.maxY();
		if (otherMinX < m_minX) m_minX = otherMinX;
		if (otherMinY < m_minY) m_minY = otherMinY;
		if (otherMaxX > m_maxX) m_maxX = otherMaxX;
		if (otherMaxY > m_maxY) m_maxY = otherMaxY;
	}

	void expandToInclude(Coordinate c)
	{
		m_minX = std::min(m_minX, c.x);
		m_maxX = std::max(m_maxX, c.x);
		m_minY = std::min(m_minY, c.y);
		m_maxY = std::max(m_maxY, c.y);
	}

	void expandToIncludeX(int32_t x)
	{
		m_minX = std::min(m_minX, x);
		m_maxX = std::max(m_maxX, x);
	}

	void setEmpty()
	{
		m_minX = std::numeric_limits<int32_t>::max();
		m_minY = std::numeric_limits<int32_t>::max();
		m_maxX = std::numeric_limits<int32_t>::min();
		m_maxY = std::numeric_limits<int32_t>::min();
	}

	void format(char* buf) const;
	std::string toString() const;

private:
	/**
	 * Overflow-safe subtraction
	 *
	 * @param x
	 * @param y
	 * @return the result of the subtraction; or the lowest negative value in case of an overflow
	 */
	static inline int trimmedSubtract(int32_t x, int32_t y)
	{
		int r = x - y;
		if (((x ^ y) & (x ^ r)) < 0) return INT_MIN;
		return r;
	}

	/**
	 * Overflow-safe addition
	 *
	 * @param x
	 * @param y
	 * @return the result of the addition; or the highest positive value in case of an overflow
	 */
	static inline int trimmedAdd(int32_t x, int32_t y)
	{
		int r = x + y;
		if (((x ^ r) & (y ^ r)) < 0) return INT_MAX;
		return r;
	}


	// Keep these in order
	int32_t m_minX;
	int32_t m_minY;
	int32_t m_maxX;
	int32_t m_maxY;
};

