// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ChangeFormatter.h"
#include "PyChangedFeature.h"
#include "PyChangedMembers.h"

void ChangeFormatter::writeFeature(clarisma::Buffer& out, int indent, PyChangedFeature* feature)
{
    static constexpr std::string_view PREFIXES[] =
        { "<node id=\"", "<way id=\"", "<relation id=\"" };
    static constexpr std::string_view CLOSING[] =
        { "</node>\n", "</way>\n", "</relation>\n" };

    out.writeRepeatedChar(' ', indent);
    out << PREFIXES[feature->type] << feature->id
        << "\" version=\"" << feature->version;
    if (feature->type == PyChangedFeature::Type::NODE)
    {
        out << "\" lon=\"" << feature->lon << "\" lat=\""
            << feature->lat;
        if (feature->tags == nullptr) [[likely]]
        {
            out << "\"/>\n";
            return;
        }
        out << "\">\n";
    }
    else
    {
        out << "\">\n";
        PyObject* list = feature->members->list;
        Py_ssize_t size = PyList_GET_SIZE(list);
        for (Py_ssize_t i = 0; i < size; ++i)
        {
            out.writeRepeatedChar(' ', indent + 2);
            PyChangedFeature* member = (PyChangedFeature*)
                PyList_GET_ITEM(list, i);
            if (feature->type == PyChangedFeature::Type::WAY)
            {
                out << "<nd ref=\"" << member->id << "\">\n";
            }
            else
            {
                PyObject* role = member->role;
                member = member->member;
                out << "<member type=\"" << typeName(
                    static_cast<FeatureType>(member->type))
                    << "\" id=\"" << member->id
                    << "\" role=\"" << Python::getStringView(role)
                    << "\">\n";
            }
        }
    }

    if (feature->tags) [[likely]]
    {
        writeTags(out, indent+2, feature->tags);
    }
    out.writeRepeatedChar(' ', indent);
    out << CLOSING[feature->type];
}


void ChangeFormatter::writeTags(clarisma::Buffer& out, int indent, PyObject* tags)
{

    out.writeRepeatedChar(' ', indent);
}