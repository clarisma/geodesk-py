#pragma once

#include "Filter.h"
#include "api/Feature.h"

template <typename Predicate>
class PredicateFilter : public Filter
{
public:
	PredicateFilter(Predicate predicate) :
		predicate_(predicate)
	{
	}

	bool accept(FeatureStore* store, FeaturePtr ptr, FastFilterHint fast) const override
	{
		geodesk::Feature feature(store, FeaturePtr(ptr.ptr()));
		return predicate_(feature);
    }

protected:
	Predicate predicate_;
};


