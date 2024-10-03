# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++17 -march=native -O3 -g -Wno-ignored-attributes

# Include directories
INCLUDES := -I ./include -I ./include/eigen -I ./include/asio/asio/include

# Libraries
LIBS := -pthread

# Windows-specific libraries required by ASIO
WIN_LIBS := -lws2_32

# Source files excluding party.cpp as it will be compiled with different macros
SRCS := ./src/client.cpp ./src/linear_regression.cpp

# Extract the base names of the source files (without extensions)
TARGETS := $(SRCS:.cpp=)

# Default target
all: $(TARGETS) party0 party1

# Compile party0 and party1 from party.cpp with respective defines
party0: ./src/party.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DPARTY0 -o $@ $< $(LIBS) $(WIN_LIBS)

party1: ./src/party.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DPARTY1 -o $@ $< $(LIBS) $(WIN_LIBS)

# Pattern rule to create an executable for each source file
$(TARGETS): % : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(WIN_LIBS)

# Clean target
clean:
	rm -f $(TARGETS) party0 party1
