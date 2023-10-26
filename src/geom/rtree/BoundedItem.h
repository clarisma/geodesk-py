// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "geom/Box.h"

template <typename T>
class BoundedItem
{
	virtual Box bounds() = 0;
	virtual const T* item() = 0;
};
