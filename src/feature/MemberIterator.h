// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/util/ShortVarString.h>
#include "feature/FeaturePtr.h"
#include "feature/FeatureStore.h"

// TODO: need to bump the refcount of `store` to ensure that store does not 
// get destroyed if the MemberIterator is used as a standalone object?

// Warning: MemberIterator cannot be used on empty relations, need to check first

// TODO: For the sake of the API, we should treat local and global strings the same

// TODO: Warning, MemberIterator is not threadsafe because it uses a Python string
// to track the role
// 4/3/24: The Python role string object is now created lazily, which means that
//  MemberIterator is now threadsafe as long as the Python role string is not 
//  accessed (via borrowCurrentRole())

class MemberIterator
{
public:
	MemberIterator(FeatureStore* store, DataPtr pMembers,
		FeatureTypes types, const MatcherHolder* matcher, const Filter* filter);

	~MemberIterator()
	{
		#ifdef GEODESK_PYTHON
		Py_XDECREF(currentRoleObject_);
		#endif
	}

	FeatureStore* store() const { return store_; }

	FeaturePtr next();
	bool isCurrentForeign() const 
	{
		return currentMember_ & MemberFlags::FOREIGN; 
	}
	Tip currentTip() const { return currentTip_; }

	std::string_view currentRole() const
	{
		if (currentRoleCode_ >= 0)
		{
			return store_->strings().getGlobalString(currentRoleCode_)->toStringView();
		}
		return currentRoleStr_->toStringView();
	}

	#ifdef GEODESK_PYTHON
	/**
	 * Obtains a borrowed reference to the Python string object that
	 * represents the role of the current member.
	 */
	PyObject* borrowCurrentRole() // const 
	{ 
		if (!currentRoleObject_)
		{
			if (currentRoleCode_ >= 0)
			{
				currentRoleObject_ = store_->strings().getStringObject(currentRoleCode_);
				assert(currentRoleObject_);
			}
			else
			{
				assert(currentRoleStr_);
				currentRoleObject_ = Python::toStringObject(*currentRoleStr_);

				//currentRoleObject_ = Python::toStringObject(
				//	currentRoleStr_->data(), currentRoleStr_->length());

				// TODO: It is possible that string creation fails if the
				//  UTF-8 encoding is bad; a GOL should never allow
				//  invalid UTF-8 data to be stored
				/*
				if(!currentRoleObject_)
				{
					printf("Bad string: %s\n  Ptr = %p\n\n",
						currentRoleStr_->toString().c_str(),
						currentRoleStr_->data());
					currentRoleObject_ = store_->strings().getStringObject(0);
					PyErr_Print();
					PyErr_Clear();
					// TODO: for debug only !!!!
				}
				*/
				assert(currentRoleObject_);
			}
			assert(currentRoleObject_);
		}
		return currentRoleObject_; 
	}
	#endif

private:
	FeatureStore* store_;
	FeatureTypes types_;
	const MatcherHolder* matcher_;
	const Filter* filter_;
	int currentRoleCode_;
	const ShortVarString* currentRoleStr_;
	#ifdef GEODESK_PYTHON
	PyObject* currentRoleObject_;
	#endif
	Tip currentTip_;
	int32_t currentMember_;
	const Matcher* currentMatcher_;
	DataPtr p_;
	DataPtr pForeignTile_;
};

