# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Include paths
INCLUDES := -I ../include -I ../include/eigen -I ../include/asio/asio/include

# Targets
TARGETS = linear_regression party0 party1

# Source files
LINEAR_REGRESSION = linear_regression.cpp
PARTY0_SRC = party0.cpp
PARTY1_SRC = party1.cpp

# Libraries for Windows networking
LIBS = -lpthread -lws2_32

# Rules
all: $(TARGETS)
linear_regression:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LINEAR_REGRESSION) -o linear_regression $(LIBS)

party0:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTY0_SRC) -o party0 $(LIBS)

party1:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTY1_SRC) -o party1 $(LIBS)

# Run commands
run_linear_regression:
	./linear_regression
	
run_party0:
	./party0

run_party1:
	./party1

# Clean up build files
clean:
	rm -f $(TARGETS)
