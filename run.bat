g++ -O2 -g -o minecraft_opengl.exe ^
main.cpp ^
./requirements/glad/glad.c ^
./requirements/imgui-1.91.6/imgui.cpp ^
./requirements/imgui-1.91.6/imgui_draw.cpp ^
./requirements/imgui-1.91.6/imgui_tables.cpp ^
./requirements/imgui-1.91.6/imgui_widgets.cpp ^
./requirements/imgui-1.91.6/backends/imgui_impl_glfw.cpp ^
./requirements/imgui-1.91.6/backends/imgui_impl_opengl3.cpp ^
-I"./requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/include" ^
-L"./requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
-I"./requirements/glm-1.0.1-light" ^
-I"./requirements/FastNoiseLite" ^
-I"./requirements/stb_image" ^
-I"./requirements/glad" ^
-I"./requirements/imgui-1.91.6/" ^
-I"./requirements/imgui-1.91.6/backends" ^
-I"./requirements/imgui-1.91.6/misc/cpp" ^
-DSTB_IMAGE_IMPLEMENTATION ^
-lglfw3 -lopengl32 -lgdi32 -lglu32
.\minecraft_opengl.exe