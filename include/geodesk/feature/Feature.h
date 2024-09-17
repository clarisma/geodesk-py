// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#ifndef GEODESK_DOXYGEN

#include "FeatureBase.h"

namespace geodesk {

using Feature = detail::FeatureBase<FeaturePtr,true,true,true>;

/*
class Feature : public detail::FeatureBase<FeaturePtr,true,true,true> // NWR
{
public:
	using FeatureBase::FeatureBase;
};
*/

} // namespace geodesk

#endif