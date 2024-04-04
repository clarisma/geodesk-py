#pragma once

#include "Filter.h"
#include "geom/Area.h"

class AreaFilter : public Filter
{
public:
	AreaFilter(double minArea, double maxArea) :
		Filter(0, minArea > 0 ? FeatureTypes::AREAS : FeatureTypes::ALL),
		minArea_(minArea),
		maxArea_(maxArea)
	{
	}

	bool accept(FeatureStore* store, FeatureRef feature, FastFilterHint fast) const override
	{
		double a;
		if (feature.isArea())
		{
			if (feature.isWay())
			{
				a = Area::ofWay(WayRef(feature));
			}
			else
			{
				assert(feature.isRelation());
				a = Area::ofRelation(store, RelationRef(feature));
			}
		}
		else
		{
			a = 0;
		}
		return a >= minArea_ && a <= maxArea_;
	}

protected:
	double minArea_;
	double maxArea_;
};


