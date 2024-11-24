# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -g

# Directories
BUILD_DIR = build

# Source files
SRC = main.cpp client.cpp commands.cpp response.cpp helper.cpp
# Object files
OBJ = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC)) $(BUILD_DIR)/message.o

# Header files
HEADER = client.hpp helper.hpp

# Output binary
TARGET = client

# Build the binary
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

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
