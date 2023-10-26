// Copyright (c) 2023 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

/*

#pragma once

#define ENUM_CLASS_BITWISE_OPERATORS(EnumType) \
inline EnumType operator|(EnumType lhs, EnumType rhs) \
{ \
    using underlying = std::underlying_type_t<EnumType>; \
    return static_cast<EnumType>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs)); \
} \
\
inline EnumType operator&(EnumType lhs, EnumType rhs) \
{ \
    using underlying = std::underlying_type_t<EnumType>; \
    return static_cast<EnumType>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs)); \
} \
\
inline EnumType& operator|=(EnumType& lhs, EnumType rhs) \
{ \
    lhs = lhs | rhs; \
    return lhs; \
} \
\
inline EnumType& operator&=(EnumType& lhs, EnumType rhs) \
{ \
    lhs = lhs & rhs; \
    return lhs; \
} \
inline operator bool(const EnumType& flags) \
{ \
    return static_cast<int>(flags) != 0; \
}

*/

