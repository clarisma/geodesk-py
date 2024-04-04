#pragma once

#include "Filter.h"
#include "geom/Length.h"

class LengthFilter : public Filter
{
public:
	LengthFilter(double minLen, double maxLen) :
		Filter(0, minLen > 0 ? (FeatureTypes::ALL & ~FeatureTypes::NODES) : FeatureTypes::ALL),
		minLen_(minLen),
		maxLen_(maxLen)
	{
	}

	bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override
	{
		double len;
		if (feature.isWay())
		{
			len = Length::ofWay(WayRef(feature));
		}
		else if(feature.isRelation())
		{
			len = Length::ofRelation(store, RelationRef(feature));
		}
		else
		{
			len = 0;
		}
		return len >= minLen_ && len <= maxLen_;
	}

protected:
	double minLen_;
	double maxLen_;
};



