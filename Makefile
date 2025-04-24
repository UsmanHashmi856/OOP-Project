# Compiler and linker settings
CXX = g++
CXXFLAGS = -I"C:/SFML-2.6.1/include" -std=c++17
LDFLAGS = -L"C:/SFML-2.6.1/lib" -lsfml-graphics -lsfml-window -lsfml-system

# Source and target files
SRC = main.cpp
OBJ = main.o
TARGET = main.exe

# Default target
all: $(TARGET)
	./$(TARGET)  # Run the compiled executable after building

# Compilation step
$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC)

# Linking step
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)
