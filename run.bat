g++ -O2 -g -o minecraft_opengl.exe main.cpp ./requirements/glad/glad.c ^
-I"./requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/include" ^
-L"./requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
-I"./requirements/glm-1.0.1-light" ^
-I"./requirements/FastNoiseLite" ^
-I"./requirements/stb_image" ^
-I"./requirements/glad" ^
-lglfw3 -lopengl32 -lgdi32 -lglu32
.\minecraft_opengl.exe

@REM -I"D:/softwares/glew-2.2.0-win32/glew-2.2.0/include" ^
@REM -L"D:/softwares/glew-2.2.0-win32/glew-2.2.0/lib/Release/x64" ^