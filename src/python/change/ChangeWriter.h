// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <clarisma/io/FileBuffer3.h>
#include "PyChangedFeature.h"

class ChangeWriter
{
public:
    explicit ChangeWriter(const char* fileName)
    {
        out_.open(fileName);
    }

    void write(Changeset* changes);

private:
    void writeFeature(PyChangedFeature* feature);
    void writeTags(PyObject* tags);
    void writeTagValue(PyObject* tags);

    struct Tag
    {
        std::string_view key;
        PyObject* value;
    };

    clarisma::FileBuffer3 out_;
    std::vector<Tag> tempTags_;
    std::vector<PyFeature*> tempFeatures_;
};
