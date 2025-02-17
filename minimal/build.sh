#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Compiler flags
CFLAGS="-Wall -Wextra -g -I./include -std=c11"
LDFLAGS="-lglfw -ldl -lm -lGL -lX11 -lpthread -lXrandr -lXi -lXxf86vm -lXcursor"

# Compile GLAD
echo "Compiling GLAD..."
gcc -c src/glad.c -o build/glad.o $CFLAGS

# Compile main program
echo "Compiling main.c..."
gcc -c src/main.c -o build/main.o $CFLAGS

# Link everything
echo "Linking..."
gcc build/glad.o build/main.o -o build/opengl_app $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful! Executable created at build/opengl_app"
else
    echo "Build failed!"
    exit 1
fi
