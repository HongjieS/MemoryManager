#ifndef ALLOCATORS_H
#define ALLOCATORS_H
#include <cstddef>
#include <climits>
#include <cstdint>
int bestFit(int sizeInWords, void *list);
int worstFit(int sizeInWords, void *list);

#endif
