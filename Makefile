# Compiler and compiler flags
CXX = g++
CXXFLAGS = -I ./include -I ./include/eigen -std=c++14

# Directories
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Source, Object, and Target files
SRCFILES = $(wildcard $(SRCDIR)/*.cpp)
OBJFILES = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCFILES))
TARGET = $(BINDIR)/main

# Default target, build the executable
all: $(TARGET)

# Linking the object files to create the final executable
$(TARGET): $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compiling each source file into an object file
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR) $(BINDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up the object and binary directories
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Run the compiled executable
run: $(TARGET)
	./$(TARGET)
