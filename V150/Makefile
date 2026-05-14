# Makefile for LightningEngine V150 - Windows Compatible
# Compiler settings
CXX = g++
DEPS_DIR ?= D:\Arquivos\HDDR\deps
DEPS_DIR_POSIX = $(subst \\,/,$(DEPS_DIR))

CXXFLAGS = -std=c++17 -Wall -Wextra -I./src/include -I$(DEPS_DIR_POSIX)/SDL3/include -I$(DEPS_DIR_POSIX)/glm/include -I$(DEPS_DIR_POSIX)/Assimp/include -I./third_party/SDL3/include -I./third_party/glm/include -I./third_party/Assimp/include
LDFLAGS = -L$(DEPS_DIR_POSIX)/SDL3/lib/x64 -L$(DEPS_DIR_POSIX)/Assimp/lib -L./third_party/SDL3/lib/x64 -L./third_party/Assimp/lib
LIBS = -lSDL3 -lassimpd -lopengl32

# Build configurations
DEBUG_FLAGS = -g -D_DEBUG
RELEASE_FLAGS = -O3 -DNDEBUG

# Source files
SRC_DIR = .
SRC_FILES = $(SRC_DIR)/Editor.cpp \
            src/core/Engine.cpp \
            src/core/Window.cpp \
            src/graphics/Renderer.cpp

# Object files
OBJ_DIR = obj
OBJ_FILES = $(OBJ_DIR)/Editor.o \
            $(OBJ_DIR)/Engine.o \
            $(OBJ_DIR)/Window.o \
            $(OBJ_DIR)/Renderer.o

# Output executable
TARGET = LightningEngine.exe

# Windows-specific commands
MKDIR = if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
RM = del /Q /F
RMDIR = rmdir /S /Q

# Default target
all: release

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Release build
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Link executable
$(TARGET): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) $(LDFLAGS) $(LIBS) -o $@

# Compile source files - Windows compatible
$(OBJ_DIR)/Editor.o: Editor.cpp
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Engine.o: src/core/Engine.cpp
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Window.o: src/core/Window.cpp
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/Renderer.o: src/graphics/Renderer.cpp
	$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts - Windows compatible
clean:
	-$(RM) $(TARGET) 2>nul
	-$(RMDIR) $(OBJ_DIR) 2>nul
	@echo Clean completed.

# Run the application
run: release
	$(TARGET)

# Install dependencies (copy DLLs)
install-dlls:
	@echo Copying required DLLs...
	@if exist "$(DEPS_DIR)\SDL3\lib\x64\*.dll" (copy "$(DEPS_DIR)\SDL3\lib\x64\*.dll" . 2>nul) else (copy "third_party\SDL3\lib\x64\*.dll" . 2>nul || echo No SDL3 DLLs to copy)
	@if exist "$(DEPS_DIR)\Assimp\lib\*.dll" (copy "$(DEPS_DIR)\Assimp\lib\*.dll" . 2>nul) else (copy "third_party\Assimp\lib\*.dll" . 2>nul || echo No Assimp DLLs to copy)

.PHONY: all debug release clean run install-dlls
