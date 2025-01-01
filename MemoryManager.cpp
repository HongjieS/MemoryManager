#include "MemoryManager.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator)
{
	this->wordSize = wordSize;
	this->allocator = allocator;
}

MemoryManager::~MemoryManager() {
    if(onCheck){
        onCheck = false;
    }
    shutdown();
    this->wordSize = 0;
    this->memorySize = 0;
}

void MemoryManager::initialize(size_t sizeInWords) {
    if (sizeInWords > 65536 || sizeInWords <0) {
            return;
        }
    memorySize = sizeInWords * wordSize;
    memoryBlock = new char[memorySize];
    bitmap.assign(sizeInWords, false);
    holes.clear();
    holes.emplace_back(0, sizeInWords);
}

void MemoryManager::shutdown() {
    delete[] memoryBlock;
    memoryBlock = nullptr;
    bitmap.clear();
    holes.clear();
    allocationSizes.clear();
}

void* MemoryManager::allocate(size_t sizeInBytes) {
    if (sizeInBytes == 0 || sizeInBytes > memorySize) {
        std::cout << "Invalid allocation request." << std::endl;
        return nullptr;
    }

    //makes sure full words are allocated
    size_t wordsNeeded = (sizeInBytes + wordSize - 1) / wordSize;

    //use allocator to find best offset for allocation
    void* list = getList();
    int wordOffset = allocator(wordsNeeded, list);
    delete[] static_cast<uint16_t*>(list);

    if (wordOffset == -1) {
        std::cout << "No hole found for allocation." << std::endl;
        return nullptr;
    }

    //calc byte offset and update memory
    char* allocatedMemory = memoryBlock + wordOffset * wordSize;
    for (size_t i = 0; i < wordsNeeded; ++i) {
        bitmap[wordOffset + i] = true;
    }

    //update allocationSizes
    allocationSizes[allocatedMemory] = wordsNeeded * wordSize;

    //adjust the holes list after allocation
    adjustHolesAfterAllocation(wordOffset, wordsNeeded);

    return allocatedMemory;
}


void MemoryManager::free(void* address) {
    if (address == nullptr || memoryBlock == nullptr) {
        std::cerr << "Tried to free a null or out-of-bounds address." << std::endl;
        return;
    }
    //calc offset
    size_t offset = static_cast<char*>(address) - memoryBlock;
    size_t wordOffset = offset / wordSize;

    auto it = allocationSizes.find(address);
    if (it == allocationSizes.end()) {
        std::cerr << "Tried to free untracked or already freed address." << std::endl;
        return;
    }

    size_t blockSize = it->second / wordSize;
    //mark bits in the bitmap as free
    for (size_t i = 0; i < blockSize; ++i) {
        bitmap[wordOffset + i] = false;
    }

    //merge adjacent holes or add a new hole
    mergeOrAddHole(wordOffset, blockSize);

    //erase the allocation record
    allocationSizes.erase(it);
}


void MemoryManager::mergeOrAddHole(size_t wordOffset, size_t blockSize) {
    bool merged = false;
    for (auto it = holes.begin(); it != holes.end(); ++it) {
        if (wordOffset + blockSize == it->first) {
            it->first -= blockSize;
            it->second += blockSize;
            merged = true;
            break;
        } else if (it->first + it->second == wordOffset) {
            it->second += blockSize;
            merged = true;
            break;
        }
    }

    if (!merged) {
        holes.emplace_back(wordOffset, blockSize);
        std::sort(holes.begin(), holes.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    }

    mergeAdjacentHoles();
}

void MemoryManager::dumpMemoryMap(const std::string& filename) {
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    for (size_t i = 0; i < holes.size(); ++i) {
        std::string holeInfo = "[" + std::to_string(holes[i].first) + ", " + std::to_string(holes[i].second) + "]";

        if (i < holes.size() - 1) {
            holeInfo += " - ";
        }

        if (write(fd, holeInfo.c_str(), holeInfo.size()) == -1) {
            std::cerr << "Failed to write to file: " << filename << std::endl;
            close(fd);
            return;
        }
    }

    close(fd);
}
void* MemoryManager::getList() {
    uint16_t lenHoles = static_cast<uint16_t>(holes.size());
    size_t listSize = static_cast<size_t>(2 * lenHoles) + 1;
    uint16_t* list = new uint16_t[listSize];
    list[0] = lenHoles;

    for (size_t i = 0, j = 1; i < lenHoles; ++i) {
        list[j++] = static_cast<uint16_t>(holes[i].first);  // Start of the hole
        list[j++] = static_cast<uint16_t>(holes[i].second); // Size of the hole
    }

    return list;
}



void MemoryManager::setAllocator(std::function<int(int, void*)> newAllocator) {
    allocator = newAllocator;
}

void* MemoryManager::getBitmap() {
    size_t totalWords = memorySize / wordSize;
    size_t bitmapByteSize = (totalWords + 7) / 8;

    //allocate memory for the bitmap
    uint8_t* bitmapArray = new uint8_t[bitmapByteSize + 2];

    //store bitmap size in the first two bytes
    bitmapArray[0] = static_cast<uint8_t>(bitmapByteSize & 0xFF);
    bitmapArray[1] = static_cast<uint8_t>((bitmapByteSize >> 8) & 0xFF);

    //init bitmap
    std::fill_n(bitmapArray + 2, bitmapByteSize, 0);

    for (size_t i = 0; i < totalWords; ++i) {
        if (bitmap[i]) {
            size_t byteIndex = i / 8 + 2;
            bitmapArray[byteIndex] |= 1 << (i % 8);
        }
    }

    return bitmapArray;
}


unsigned MemoryManager::getWordSize() {
    return wordSize;
}

void* MemoryManager::getMemoryStart() {
    if (memoryBlock){
        return memoryBlock;
    }
    return nullptr;
}

unsigned MemoryManager::getMemoryLimit() {
    return memorySize;
}

void MemoryManager::adjustHolesAfterAllocation(int wordOffset, size_t wordsNeeded) {
    std::vector<std::pair<size_t, size_t>> newHoles;
    bool holeAdded = false;

    for (auto& hole : holes) {
        if (wordOffset == hole.first && wordsNeeded == hole.second) {
            holeAdded = true;
        } else if (wordOffset <= hole.first + hole.second && wordOffset >= hole.first) {
            if (wordOffset > hole.first) {
                //add left part of the split hole
                newHoles.emplace_back(hole.first, wordOffset - hole.first);
            }
            if (wordOffset + wordsNeeded < hole.first + hole.second) {
                //add right part of the split hole
                newHoles.emplace_back(wordOffset + wordsNeeded, hole.first + hole.second - (wordOffset + wordsNeeded));
            }
            holeAdded = true;
        } else {
            newHoles.push_back(hole);
        }
    }

    if (!holeAdded) {
        newHoles.emplace_back(wordOffset, wordsNeeded);
    }

    holes = newHoles;
    std::sort(holes.begin(), holes.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    mergeAdjacentHoles();
}



void MemoryManager::mergeAdjacentHoles() {
    for (size_t i = 0; i + 1 < holes.size(); ) {
        if (holes[i].first + holes[i].second >= holes[i + 1].first) {
            holes[i].second += holes[i + 1].second;
            holes.erase(holes.begin() + i + 1);
        } else {
            ++i;
        }
    }
}



