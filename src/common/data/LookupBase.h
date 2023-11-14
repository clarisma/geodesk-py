#pragma once
#include <cstdint>
#include <cstring>

/**
 * Base template for classes that use hashtable-based lookup.
 * Items whose hashes collide are chained using an intrusive linked list,
 * i.e. the pointers to the next item are stored in each item itself.
 * 
 * The hashtable itself is stored externally. Its lifetime is managed
 * by the user, and its pointer and size (number of slots) must be provided
 * to the constructor. (The constructor does, however, initialize the table.)
 * 
 * The following functions must be provided by the derived class:
 * 
 * - static T** next(T* item)
 *   Returns the location of the "next item" pointer in the given item
 */
template<typename Derived, typename T>
class LookupBase
{
public:
	LookupBase() :
		table_(nullptr),
		tableSize_(0)
	{
	}

	void init(T** table, size_t tableSize)
	{
		table_ = table;
		tableSize_ = tableSize;
		memset(table, 0, sizeof(T*) * tableSize);
	}

	class Iterator
	{
	public:
		Iterator(const LookupBase* lookup)
		{
			pCurrentSlot_ = table_;
			pEndSlot_ = table_ + tableSize_;
			nextSlot();
		}
		
		T* next()
		{
			T* p = pCurrentItem_;
			pCurrentItem_ = *Derived::next(p);
			if (!pCurrentItem_)
			{
				pCurrentSlot_++;
				nextSlot();
			}
			return p;
		}

	private:
		void nextSlot()
		{
			while(pCurrentSlot_ != pEndSlot_)
			{
				pCurrentItem_ = *pCurrentSlot_;
				if (!pCurrentItem_) break;
				pCurrentSlot_++;
			}
		}

		T** pCurrentSlot_;
		T** pEndSlot_;
		T* pCurrentItem_;
	};

	Iterator iter() const
	{
		return Iterator(this);
	}

	void copyItems(T** pa)
	{
		Iterator iter(this);
		for (;;)
		{
			T* p = iter.next();
			if (!p) break;
			*pa++ = p;
		}
	}

protected:

	T** table_;
	size_t tableSize_;
};
