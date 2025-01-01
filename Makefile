# Compiler and flags
CXX = g++
AR = ar
CXXFLAGS = -O2 -g -pipe -DSTB_IMAGE_IMPLEMENTATION
INCLUDES = -I"./requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/include" \
          -I"./requirements/glm-1.0.1-light" \
          -I"./requirements/FastNoiseLite" \
          -I"./requirements/stb_image" \
          -I"./requirements/glad" \
          -I"./requirements/imgui-1.91.6/" \
          -I"./requirements/imgui-1.91.6/backends"

LIBS = -L"./requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" \
       -lglfw3 -lopengl32 -lgdi32 -lglu32

# Source files
GLAD_SRC = requirements/glad/glad.c
IMGUI_SRCS = requirements/imgui-1.91.6/imgui.cpp \
             requirements/imgui-1.91.6/imgui_draw.cpp \
             requirements/imgui-1.91.6/imgui_tables.cpp \
             requirements/imgui-1.91.6/imgui_widgets.cpp \
             requirements/imgui-1.91.6/backends/imgui_impl_glfw.cpp \
             requirements/imgui-1.91.6/backends/imgui_impl_opengl3.cpp
GAME_SRCS = main.cpp

# Object files
GLAD_OBJ = $(GLAD_SRC:.c=.o)
IMGUI_OBJS = $(IMGUI_SRCS:.cpp=.o)
GAME_OBJS = $(GAME_SRCS:.cpp=.o)

# Library files
GLAD_LIB = lib/libglad.a
IMGUI_LIB = lib/libimgui.a

# Output executable
TARGET = minecraft_opengl.exe

.DEFAULT_GOAL := all
# Default parallel execution
# MAKEFLAGS += -j

# Rules
.PHONY: all clean directories

all: directories 
	make -j $(TARGET)

directories:
	@if not exist lib mkdir lib

$(GLAD_LIB): $(GLAD_OBJ)
	$(AR) rcs $@ $^

$(IMGUI_LIB): $(IMGUI_OBJS)
	$(AR) rcs $@ $^

$(TARGET): $(GAME_OBJS) $(GLAD_LIB) $(IMGUI_LIB)
	$(CXX) $(GAME_OBJS) $(GLAD_LIB) $(IMGUI_LIB) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@if exist requirements\glad\glad.o del requirements\glad\glad.o
	@if exist requirements\imgui-1.91.6\imgui.o del requirements\imgui-1.91.6\imgui.o
	@if exist requirements\imgui-1.91.6\imgui_draw.o del requirements\imgui-1.91.6\imgui_draw.o
	@if exist requirements\imgui-1.91.6\imgui_tables.o del requirements\imgui-1.91.6\imgui_tables.o
	@if exist requirements\imgui-1.91.6\imgui_widgets.o del requirements\imgui-1.91.6\imgui_widgets.o
	@if exist requirements\imgui-1.91.6\backends\imgui_impl_glfw.o del requirements\imgui-1.91.6\backends\imgui_impl_glfw.o
	@if exist requirements\imgui-1.91.6\backends\imgui_impl_opengl3.o del requirements\imgui-1.91.6\backends\imgui_impl_opengl3.o
	@if exist main.o del main.o
	@if exist minecraft_opengl.exe del minecraft_opengl.exe
	@if exist lib\libglad.a del lib\libglad.a
	@if exist lib\libimgui.a del lib\libimgui.a
	@if exist lib rmdir /s /q lib

run: all
	.\$(TARGET)
