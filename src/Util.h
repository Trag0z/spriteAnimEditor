#pragma once
#include "pch.h"

template <typename T> inline T greatest_common_divisor(T a, T b) {
    if (b == 0)
        return a;
    return greatest_common_divisor(b, a % b);
}