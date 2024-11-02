# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Include paths
INCLUDES := -I ../../include -I ../../include/eigen -I ../../include/asio/asio/include

# Targets
TARGETS = party0 party1

# Source files
PARTY0_SRC = party0.cpp
PARTY1_SRC = party1.cpp

# Libraries for Windows networking
LIBS = -lpthread -lws2_32

# Rules
all: $(TARGETS)
party0:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTY0_SRC) -o party0 $(LIBS)

party1:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTY1_SRC) -o party1 $(LIBS)

# Run commands
run_party0:
	./party0

run_party1:
	./party1

# Clean up build files
clean:
	rm -f $(TARGETS)
