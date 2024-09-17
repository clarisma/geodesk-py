// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "feature/FeaturePtr.h"

namespace geodesk::detail {

template <typename Q>
class QueryAdapter
{
public:
    template <typename... Args>
    explicit QueryAdapter(Args&&... args) : query_(std::forward<Args>(args)...) {}
    virtual ~QueryAdapter() {}
    virtual FeaturePtr next() = 0;

    FeatureStore* store() const { return query_.store(); }

protected:
    Q query_;
};

} // namespace geodesk::detail
