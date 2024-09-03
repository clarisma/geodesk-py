// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <common/text/Format.h>
#include "feature/Tex.h"
#include "feature/Tip.h"

struct ForeignFeatureRef
{
	ForeignFeatureRef(Tip tip_, Tex tex_) : tip(tip_), tex(tex_) {}
	ForeignFeatureRef() : tip(0), tex(0) {}

	bool isNull() const { return tip.isNull(); }

	void format(char* buf) const
	{
		tip.format(buf);
		buf[6] = '#';
		Format::integer(&buf[7], tex);
	}

	std::string toString() const
	{
		char buf[32];
		format(buf);
		return std::string(buf);
	}

	bool operator==(const ForeignFeatureRef& other) const
	{
		return tip == other.tip && tex == other.tex;
	}

	Tip tip;
	Tex tex;
};

