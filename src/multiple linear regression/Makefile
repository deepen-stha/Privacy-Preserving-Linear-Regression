# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Include paths
INCLUDES := -I ../../include -I ../../include/eigen -I ../../include/asio/asio/include

# Targets
TARGETS = linear_regression

# Source files
LINEAR_REGRESSION = linear_regression.cpp

# Libraries for Windows networking
LIBS = -lpthread -lws2_32

# Rules
all: $(TARGETS)
linear_regression:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LINEAR_REGRESSION) -o linear_regression $(LIBS)

# Run commands
run_linear_regression:
	./linear_regression

# Clean up build files
clean:
	rm -f $(TARGETS)
