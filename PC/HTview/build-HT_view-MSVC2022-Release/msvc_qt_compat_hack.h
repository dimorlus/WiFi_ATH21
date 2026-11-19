
#pragma once
#include <iterator>
namespace std {
    template<typename T>
    inline T* make_checked_array_iterator(T* p, size_t, size_t = 0) {
        return p;
    }
}
