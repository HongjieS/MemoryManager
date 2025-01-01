#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <functional>
#include <vector>
#include <map>
#include <cstdint>
#include "Allocators.h"
class MemoryManager {
private:
    unsigned wordSize;
    char* memoryBlock;
    size_t memorySize;
    //finding memory hole
    std::function<int(int, void*)> allocator;
    //tracking memory allocation
    std::vector<bool> bitmap;
    //track size of allocations
    std::map<void*, size_t> allocationSizes;
    //track free memory holes
    std::vector<std::pair<size_t, size_t>> holes;
    bool onCheck = false;
    void mergeAdjacentHoles();


public:
    MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
        ~MemoryManager();

        void initialize(size_t sizeInWords);
        void shutdown();
        void* allocate(size_t sizeInBytes);
        void free(void* address);
        void setAllocator(std::function<int(int, void*)> newAllocator);
        void dumpMemoryMap(const std::string& filename);
        void* getList();
        void* getBitmap();
        unsigned getWordSize();
        void* getMemoryStart();
        unsigned getMemoryLimit();
        std::string generateMemoryMapInfo();
        void adjustHolesAfterAllocation(int wordOffset, size_t wordsNeeded);
        void mergeOrAddHole(size_t wordOffset, size_t blockSize);
};

#endif
