# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Include paths
INCLUDES := -I ../include -I ../include/eigen -I ../include/asio/asio/include

# Targets
TARGETS = client party0 party1

# Source files
CLIENT_SRC = client.cpp
PARTY0_SRC = party0.cpp
PARTY1_SRC = party1.cpp

# Libraries for Windows networking
LIBS = -lpthread -lws2_32

# Rules
all: $(TARGETS)

client:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(CLIENT_SRC) -o client $(LIBS)

party0:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTY0_SRC) -o party0 $(LIBS)

party1:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTY1_SRC) -o party1 $(LIBS)

# Run commands
run_client:
	./client

run_party0:
	./party0

run_party1:
	./party1

# Run all (open separate terminals or background them to run concurrently)
run_all:
	@echo "Starting Party1..."
	./party1 &
	@echo "Starting Party0..."
	./party0 &
	sleep 1  # Small delay to ensure parties are running
	@echo "Starting Client..."
	./client

# Clean up build files
clean:
	rm -f $(TARGETS)
