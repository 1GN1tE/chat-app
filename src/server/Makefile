# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -g
LDFLAGS = -lsqlite3

# Directories
BUILD_DIR = build

# Source files
SRC = main.cpp \
server.cpp threadpool.cpp handlers.cpp\
helper.cpp channel.cpp client.cpp \
database.cpp db_pool.cpp

# Object files
OBJ = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC)) $(BUILD_DIR)/message.o

# Header files
HEADER = server.hpp threadpool.hpp \
helper.hpp client.hpp channel.hpp \
database.hpp db_pool.hpp

# Output binary
TARGET = server

# Build the binary
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile .cpp files into .o object files in the build directory
$(BUILD_DIR)/%.o: %.cpp $(HEADER) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile message.o specifically in the build directory
$(BUILD_DIR)/message.o: ../protocol/message.cpp ../protocol/message.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c ../protocol/message.cpp -o $(BUILD_DIR)/message.o

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJ)
	rm -rf $(BUILD_DIR)
