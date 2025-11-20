// Copyright (c) 2025 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#include "ChangeWriter.h"
#include <clarisma/util/Xml.h>
#include "python/version.h"
#include "Changeset.h"

using namespace clarisma;

void ChangeWriter::write(Changeset* changes)
{
    out_ << "<osmChange version=\"0.6\" generator=\"geodesk-py/" GEODESK_PY_VERSION "\">\n";

    out_ << "  <modify>\n";
    for (int i=0; i<3; i++)
    {
        for (auto& entry : changes->existing_[i])
        {
            writeFeature(entry.second.get());
        }
    }
    out_ << "  </modify>\n";

    out_ << "</osmChange>\n";
}

void ChangeWriter::writeFeature(PyChangedFeature* feature)
{
    static constexpr std::string_view PREFIXES[] =
        { "<node id=\"", "<way id=\"", "<relation id=\"" };
    static constexpr std::string_view CLOSING[] =
        { "</node>\n", "</way>\n", "</relation>\n" };

    // TODO !!!
    if (feature->version() == 0) feature->setVersion(1);

    out_ << "    " << PREFIXES[feature->type()] << feature->id()
        << "\" version=\"" << feature->version();
    if (feature->type() == PyChangedFeature::Type::NODE)
    {
        out_ << "\" lon=\"" << feature->lon()
            << "\" lat=\"" << feature->lat();
        if (!feature->hasTags()) [[likely]]
        {
            out_ << "\"/>\n";
            return;
        }
        out_ << "\">\n";
    }
    else
    {
        out_ << "\">\n";
        PyObject* list = feature->children();
        Py_ssize_t size = PyList_GET_SIZE(list);
        for (Py_ssize_t i = 0; i < size; ++i)
        {
            PyChangedFeature* member = (PyChangedFeature*)
                PyList_GET_ITEM(list, i);
            if (feature->type() == PyChangedFeature::Type::WAY)
            {
                out_ << "      <nd ref=\"" << member->id() << "\"/>\n";
            }
            else
            {
                PyObject* role = member->role();
                member = member->member();
                out_ << "      <member type=\""
                    << typeName(member->featureType())
                    << "\" id=\"" << member->id()
                    << "\" role=\"" << Python::getStringView(role)
                    << "\"/>\n";
            }
        }
    }

    if (feature->tags()) [[likely]]
    {
        writeTags(feature->tags());
    }
    out_ << "    " << CLOSING[feature->type()];
}


void ChangeWriter::writeTags(PyObject* tags)
{
    Py_ssize_t pos = 0;               // iteration state
    PyObject* key = nullptr;          // borrowed
    PyObject* value = nullptr;        // borrowed

    while (PyDict_Next(tags, &pos, &key, &value))
    {
        tempTags_.emplace_back(Python::getStringView(key), value);
    }
    std::ranges::sort(tempTags_, std::less<>{}, &Tag::key);

    for (Tag tag : tempTags_)
    {
        out_ << "      <tag k=\"" << tag.key << "\" v=\"";
        writeTagValue(tag.value);
        out_ << "\"/>\n";
    }
}


void ChangeWriter::writeTagValue(PyObject* value)
{
    if (PyUnicode_Check(value)) [[likely]]
    {
        Xml::writeEscaped(out_, Python::stringAsStringView(value));
        // TODO: could fail
        return;
    }
    if (PyBool_Check(value))
    {
        const std::string_view s = (value == Py_True) ? "yes" : "no";
        out_ << s;
        return;
    }

}