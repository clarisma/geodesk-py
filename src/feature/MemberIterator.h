// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "Feature.h"
#include "FeatureStore.h"

// TODO: need to bump the refcount of `store` to ensure that store does not 
// get destroyed if the MemberIterator is used as a standalone object?

// Warning: MemberIterator cannot be used on empty relations, need to check first

class MemberIterator
{
public:
	MemberIterator(FeatureStore* store, pointer pMembers,
		FeatureTypes types, const MatcherHolder* matcher, const Filter* filter);

	~MemberIterator()
	{
		Py_DECREF(currentRoleObject_);
	}

	FeatureStore* store() const { return store_; }

	FeatureRef next();
	bool isCurrentForeign() const 
	{
		return currentMember_ & MemberFlags::FOREIGN; 
	}
	Tip currentTip() const { return currentTip_; }

	/**
	 * Obtains a borrowed reference to the Python string object that
	 * represents the role of the current member.
	 */
	PyObject* borrowCurrentRole() const { return currentRoleObject_; }

private:
	FeatureStore* store_;
	FeatureTypes types_;
	const MatcherHolder* matcher_;
	const Filter* filter_;
	int currentRoleCode_;
	LocalString currentRoleStr_;
	PyObject* currentRoleObject_;
	Tip currentTip_;
	int32_t currentMember_;
	const Matcher* currentMatcher_;
	pointer p_;
	pointer pForeignTile_;
};


/*

class AllMemberIterator : public MemberIterator
{
public:
	AllMemberIterator(FeatureStore* store, pointer pMembers) :
		MemberIterator(store, pMembers, FeatureTypes::ALL,
			store->borrowAllMatcher(), nullptr) {}
};

*/
