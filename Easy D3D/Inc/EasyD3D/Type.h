#pragma once

// ===== primitive types =====
/* byte */ using byte = unsigned char;
/* sbyte */ using sbyte = signed char;
/* int8 */ using int8 = sbyte;
/* uint8 */ using uint8 = byte;

/* short */
/* ushort */ using ushort = unsigned short;
/* int16 */ using int16 = short;
/* uint16 */ using uint16 = ushort;

/* int */
/* uint */ using uint = unsigned int;
/* long */
/* ulong */ using ulong = unsigned long;
/* int32 */ using int32 = int;
/* uint32 */ using uint32 = uint;

/* llong */ using llong = long long;
/* ullong */ using ullong = unsigned long long;
/* int64 */ using int64 = llong;
/* uint64 */ using uint64 = ullong;

/* intptr */ using intptr = intptr_t;
/* uintptr */ using uintptr = uintptr_t;

/* char */
/* wchar */ using wchar = wchar_t;


// ===== pointers =====
#include <memory>
/* Holder<T> */ template <typename T> using Holder = std::unique_ptr<T>;
/* Ptr<T> */ template <typename T> using Ptr = std::shared_ptr<T>;
/* WeakPtr<T> */ template <typename T> using WeakPtr = std::weak_ptr<T>;

#include <atlbase.h>
/* ComPtr<T> */ template <typename T> using ComPtr = CComPtr<T>;


