// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"
#include "FeatureStore.h"

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
	MemberIterator(FeatureStore* store, pointer pMembers,
		FeatureTypes types, const MatcherHolder* matcher, const Filter* filter);

	~MemberIterator()
	{
		#ifdef GEODESK_PYTHON
		Py_XDECREF(currentRoleObject_);
		#endif
	}

	FeatureStore* store() const { return store_; }

	FeatureRef next();
	bool isCurrentForeign() const 
	{
		return currentMember_ & MemberFlags::FOREIGN; 
	}
	Tip currentTip() const { return currentTip_; }

	std::string_view currentRole() const
	{
		if (currentRoleCode_ >= 0)
		{
			return store_->strings().getGlobalString(currentRoleCode_).toStringView();
		}
		else
		{
			return currentRoleStr_.toStringView();
		}
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
			}
			else
			{
				currentRoleObject_ = currentRoleStr_.toStringObject();
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
	LocalString currentRoleStr_;
	#ifdef GEODESK_PYTHON
	PyObject* currentRoleObject_;
	#endif
	Tip currentTip_;
	int32_t currentMember_;
	const Matcher* currentMatcher_;
	pointer p_;
	pointer pForeignTile_;
};

