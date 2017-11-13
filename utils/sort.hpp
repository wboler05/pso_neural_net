#ifndef SORT_HPP
#define SORT_HPP

#include <vector>

namespace CustomSort {

template <class T>
size_t partition(std::vector<T> & v, const size_t & left, const size_t & right) {

    // Find a good partition (average of 3 points);
    size_t m = (right + left) / 2;
    T p = (v[left] + v[right] + v[m]) / 3.0;

    // Run the quick sort partition
    size_t i = left;
    size_t j = right;
    while (i < j) {
        while (v[i] <= p && i <= j) i++;
        while (v[j] > p && j >= i) j--;
        if (i < j)
            std::swap(v[i], v[j]);
    }
    return i;
}

template <class T>
void insertionSort(std::vector<T> & v) {
    for (size_t i = 1; i < v.size(); i++) {
        T x = v[i];
        size_t xIt = i-1;
        while (v[xIt] > x) {
            v[xIt + 1] = v[xIt];
            if (xIt != 0)
                xIt--;
            else
                break;
        }
        v[xIt+1] = x;
    }
}

template <class T>
void quickSort(std::vector<T> & v, const size_t & left, const size_t & right) {
    if (left >= right) {
        return;
    } else {
        if (right-left > 10) {
            size_t p = partition(v, left, right-1);
            quickSort(v, left, p);
            quickSort(v, p + 1, right);
        } else {
            insertionSort(v);
        }
    }
}

template <class T>
void quickSort(std::vector<T> & v) {
    quickSort(v, 0, v.size());
}

}

#endif // SORT_HPP
