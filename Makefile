.PHONY: all clean test format

# Detect compiler and set platform-specific flags
UNAME_S := $(shell uname -s)
CXX := c++
CXX_VERSION := $(shell $(CXX) --version)

# Check if we're using Apple clang
ifneq (,$(findstring Apple,$(CXX_VERSION)))
    EXTRA_FLAGS := -undefined dynamic_lookup
else
    EXTRA_FLAGS :=
endif

# Get Python extension suffix
PYTHON_EXT_SUFFIX := $(shell python3-config --extension-suffix)
TARGET := cpp_boggle$(PYTHON_EXT_SUFFIX)

# Get pybind11 includes, replacing -I with -isystem
PYBIND11_INCLUDES := $(shell uv run python -m pybind11 --includes | perl -pe 's/-I/-isystem /g')

# Compiler flags
CXXFLAGS := -Wall -std=c++20 -fPIC -march=native \
            -Wno-sign-compare -Wshadow -Werror -O3

# Source files
SOURCES := cpp/cpp_boggle.cc cpp/trie.cc cpp/compact_trie.cc
HEADERS := $(wildcard cpp/*.h)

# Default target
all: $(TARGET)

# Build the Python extension
$(TARGET): $(SOURCES) $(HEADERS)
	@echo "Building $(TARGET)..."
	$(CXX) -shared $(CXXFLAGS) $(PYBIND11_INCLUDES) $(SOURCES) -o $(TARGET) $(EXTRA_FLAGS)
	@echo "Build complete!"

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -f cpp/*.o
	@echo "Cleaned build artifacts"

# Run tests
test: $(TARGET)
	uv run pytest boggle/ -v

# Format code
format:
	ruff format boggle/
	clang-format -i cpp/*.h cpp/*.cc
	@echo "Code formatted"

# Show build configuration
config:
	@echo "CXX: $(CXX)"
	@echo "CXX_VERSION: $(CXX_VERSION)"
	@echo "EXTRA_FLAGS: $(EXTRA_FLAGS)"
	@echo "TARGET: $(TARGET)"
	@echo "PYBIND11_INCLUDES: $(PYBIND11_INCLUDES)"
