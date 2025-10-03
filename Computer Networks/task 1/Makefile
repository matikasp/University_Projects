# Peer Time Synchronization Project Makefile

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2

# Project files
SRCS = $(wildcard *.cpp)
HEADERS = $(wildcard *.hpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
TARGET = peer-time-sync

# Default target
.PHONY: all
all: $(TARGET)

# Build the target binary
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generate dependencies automatically
-include $(OBJS:.o=.d)

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst %.cpp,%.o,$<)' $< > $@

# Clean object files and binaries
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) $(OBJS:.o=.d)

# Rebuild everything
.PHONY: rebuild
rebuild: clean all

# Run the program
.PHONY: run
run: all
	./$(TARGET)

# Display help
.PHONY: help
help:
	@echo "====== Peer Time Synchronization Makefile ======"
	@echo "Available targets:"
	@echo "  all       - Build the project (default)"
	@echo "  clean     - Remove all build artifacts"
	@echo "  rebuild   - Clean and rebuild the project"
	@echo "  run       - Build and run the program"
	@echo "  help      - Display this help message"
