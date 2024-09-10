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

	bool accept(FeatureStore* store, FeaturePtr feature, FastFilterHint fast) const override
	{
		double len;
		if (feature.isWay())
		{
			len = Length::ofWay(WayPtr(feature));
		}
		else if(feature.isRelation())
		{
			len = Length::ofRelation(store, RelationPtr(feature));
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



