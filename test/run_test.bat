g++ -O2 -g -o test.exe test.cpp ^
-I"../requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/include" ^
-L"../requirements/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-mingw-w64" ^
-L"../requirements/freeglut-MinGW-3.0.0-1.mp/freeglut/lib/x64" ^
-I"../requirements/freeglut-MinGW-3.0.0-1.mp/freeglut/include" ^
-I"../requirements/glew-2.1.0/glew-2.1.0/include" ^
-L"../requirements/glew-2.1.0/glew-2.1.0/lib" ^
-I"../requirements/FastNoiseLite" ^
-I"../requirements/stb_image" ^
-lglfw3 -lopengl32 -lgdi32 -lglu32 -lglew32
.\test.exe

@REM -I"D:/softwares/glew-2.2.0-win32/glew-2.2.0/include" ^
@REM -L"D:/softwares/glew-2.2.0-win32/glew-2.2.0/lib/Release/x64" ^