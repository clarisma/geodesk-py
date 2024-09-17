#pragma once

#include "Filter.h"
#include <geodesk/feature/Feature.h>

template <typename Predicate>
class PredicateFilter : public Filter
{
public:
	explicit PredicateFilter(Predicate predicate) :
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


