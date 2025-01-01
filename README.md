Name
Project 2: Memory Management

Synopsis
Implemented a memory manager in C++ with features such as initializing, tracking, allocating, and deallocating memory sections.

Description

Filepaths and Files Edited:

/home/reptilian/MemoryManager/Allocators.cpp
/home/reptilian/MemoryManager/MemoryManager.cpp
/home/reptilian/MemoryManager/Makefile
Tracking and Data Structures:
Utilized a vector of pairs to track memory holes and a map for allocated spaces. The vector tracks where each hole starts and its size, while the map links allocated space start points to their sizes. This structure efficiently manages memory usage and freeing.

Allocation Functions:

allocate: Requests memory blocks of a given size, calculates word alignment, and allocates memory.
free: Frees previously allocated memory at a specific address.
bestFit: Selects the smallest hole that fits the requested size to minimize wasted space.
worstFit: Selects the largest available hole to leave the most usable space for future allocations.
Key Functionality:

MemoryManager Constructor: Initializes with word size and allocation strategy.
MemoryManager Destructor: Cleans up resources to prevent memory leaks.
initialize(size_t sizeInWords): Sets up the memory block as entirely free.
shutdown(): Releases the memory block and resets internal structures.
getList(): Returns a list of free memory holes.
getBitmap(): Generates a bitmap of memory allocation.
setAllocator(std::function<int(int, void*)> newAllocator): Changes the allocation strategy.
dumpMemoryMap(const std::string& filename): Outputs the current memory map to a file.
adjustHolesAfterAllocation: Updates the holes list after allocation.
mergeOrAddHole: Merges or adds new holes after freeing memory.
mergeAdjacentHoles: Optimizes space by combining neighboring free holes.
POSIX Calls:

open: Opens a file descriptor.
write: Writes data to the file.
close: Closes the file descriptor to prevent leaks.
Testing
Testing was conducted using Valgrind and manual code reviews to ensure all memory allocations and deallocations were handled correctly. The valgrind ./CommandLineTest command was used alongside GDB for debugging.

Bugs
Encountered issues with improper alignment of requested sizes with word sizes, leading to miscalculations in required words.

Screencast Link
https://youtu.be/ohcLrM7FHs8

References/Citations

Project 2 Instruction PDF
Discussion video
Author
Hongjie Shi
