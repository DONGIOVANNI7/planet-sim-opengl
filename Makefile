# Compiler to use
CXX = g++

# Libraries to link 
LIBS = -lGL -lGLU -lglut -lm

# Name of my program(executable)
TARGET = planet_project

# My source file
SRC = main.cpp

all: $(TARGET)

# Rule to compile
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(LIBS)

# Rule to run the program
run: $(TARGET)
	./$(TARGET)

# Rule to clean up
clean:
	rm -f $(TARGET)