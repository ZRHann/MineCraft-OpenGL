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


# Environment detection
ifeq ($(MSYSTEM),UCRT64)
    IS_WINDOWS = 0
else
    IS_WINDOWS = 1
endif

# Rules
.PHONY: all clean directories

all: directories clean
	@echo Operating System detected: $(OS)
	@echo Is Windows: $(IS_WINDOWS)
	$(MAKE) -j $(TARGET)

directories:
ifeq ($(IS_WINDOWS), 1)
	@if not exist lib (mkdir lib)
else
	@mkdir -p lib
endif

$(GLAD_LIB): $(GLAD_OBJ)
ifeq ($(IS_WINDOWS), 1)
	@if not exist lib (mkdir lib)
else
	@mkdir -p lib
endif
	$(AR) rcs $@ $^

$(IMGUI_LIB): $(IMGUI_OBJS)
ifeq ($(IS_WINDOWS), 1)
	@if not exist lib (mkdir lib)
else
	@mkdir -p lib
endif
	$(AR) rcs $@ $^

$(TARGET): $(GAME_OBJS) $(GLAD_LIB) $(IMGUI_LIB)
	$(CXX) $(GAME_OBJS) $(GLAD_LIB) $(IMGUI_LIB) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
ifeq ($(IS_WINDOWS), 1)
	@if exist *.o del /Q *.o
	@if exist $(TARGET) del /Q $(TARGET)
	@if exist lib rmdir /S /Q lib
else
	rm -f $(GLAD_OBJ) $(IMGUI_OBJS) $(GAME_OBJS) $(TARGET)
	rm -f $(GLAD_LIB) $(IMGUI_LIB)
	rm -rf lib
endif

run: all
ifeq ($(IS_WINDOWS), 1)
	.\$(TARGET)
else
	./$(TARGET)
endif
