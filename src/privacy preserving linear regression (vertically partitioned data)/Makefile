# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Include paths
INCLUDES := -I ../../include -I ../../include/eigen -I ../../include/asio/asio/include

# Targets
TARGETS = privacy_preserving_multiple_linear_regression

# Source files
PRIVACY_LINEAR_REGRESSION = privacy_preserving_multiple_linear_regression.cpp

# Libraries for Windows networking
LIBS = -lpthread -lws2_32

# Rules
all: $(TARGETS)
privacy_preserving_multiple_linear_regression:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PRIVACY_LINEAR_REGRESSION) -o privacy_preserving_multiple_linear_regression $(LIBS)

# Run commands
run_privacy_preserving_multiple_linear_regression:
	./privacy_preserving_multiple_linear_regression

# Clean up build files
clean:
	rm -f $(TARGETS)
