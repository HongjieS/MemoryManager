# Compiler settings
CXX = g++
CXXFLAGS = -Wall -c

# Source files
SOURCES = MemoryManager.cpp Allocators.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LIBRARY = libMemoryManager.a

# target
all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(LIBRARY)

