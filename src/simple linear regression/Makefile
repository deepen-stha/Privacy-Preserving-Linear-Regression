# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Include paths
INCLUDES := -I ../../include -I ../../include/eigen -I ../../include/asio/asio/include

# Targets
TARGETS = simple_linear_regression

# Source files
SIMPLE_LINEAR_REGRESSION = simple_linear_regression.cpp

# Libraries for Windows networking
LIBS = -lpthread -lws2_32

# Rules
all: $(TARGETS)
simple_linear_regression:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SIMPLE_LINEAR_REGRESSION) -o simple_linear_regression $(LIBS)

# Run commands
run_simple_linear_regression:
	./simple_linear_regression

# Clean up build files
clean:
	rm -f $(TARGETS)
