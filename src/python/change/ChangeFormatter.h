// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <clarisma/util/Buffer.h>

class PyChanges;
class PyChangedFeature;

class ChangeFormatter
{
public:
    static void writeFeature(clarisma::Buffer& out, int indent, PyChangedFeature* feature);
    static void writeTags(clarisma::Buffer& out, int indent, PyObject* tags);

private:
};
