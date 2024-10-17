.PHONY: clean platform.exe game.dll all

COMPARGS := -g -O0 -Wno-deprecated-declarations -Wno-backslash-newline-escape 

DATE := $(shell date +"%H%M%S")
GAME_DLL_NAME = build/game_$(DATE).dll
PLATFORM_NAME = build/platform_$(DATE).exe

all: platform.exe game.dll

platform.exe: build/glad.lib build/glfw.lib build/build.txt
	@clang code/win32_platform.cpp $(COMPARGS) -I code -I external/glad/include -I external/GLFW/include -g -O0 -o $(PLATFORM_NAME) build/glfw.lib build/glad.lib -luser32 -lgdi32 -lshell32 -lopengl32
	-mv $(PLATFORM_NAME) platform.exe

game.dll: build/build.txt
	@clang code/game.cpp $(COMPARGS) -I code -shared -o $(GAME_DLL_NAME) -O0 -g
	@mv $(GAME_DLL_NAME) game.dll

build/build.txt:
	@mkdir build
	@touch build/build.txt

build/glad.lib: external/glad/src/glad.c build/build.txt
	@clang external/glad/src/glad.c -I external/glad/include -c -o build/glad.o
	@ar -r build/glad.lib build/glad.o

build/glfw.lib: build/build.txt
	@clang -g external/glfw/src/init.c -c -o build/glfw_init.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/context.c -c -o build/glfw_context.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/input.c -c -o build/glfw_input.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/monitor.c -c -o build/glfw_monitor.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/window.c -c -o build/glfw_window.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/vulkan.c -c -o build/glfw_vulkan.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/osmesa_context.c -c -o build/osmesa_context.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/egl_context.c -c -o build/egl_context.o -I external/glfw/include -D _GLFW_WIN32
	@clang -d external/glfw/src/wgl_context.c -c -o build/wgl_context.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/win32_joystick.c -c -o build/win32_joystick.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/win32_init.c -c -o build/win32_init.o -I external/glfw/include -D _GLFW_WIN32
	@clang -d external/glfw/src/win32_monitor.c -c -o build/win32_monitor.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/win32_thread.c -c -o build/win32_thread.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/win32_time.c -c -o build/win32_time.o -I external/glfw/include -D _GLFW_WIN32
	@clang -g external/glfw/src/win32_window.c -c -o build/win32_window.o -I external/glfw/include -D _GLFW_WIN32
	@ar -r build/glfw.lib build/glfw_init.o build/glfw_context.o build/glfw_input.o build/glfw_monitor.o build/glfw_window.o build/glfw_vulkan.o build/osmesa_context.o build/egl_context.o build/wgl_context.o build/win32_joystick.o build/win32_init.o build/win32_monitor.o build/win32_thread.o build/win32_time.o build/win32_window.o

clean:
	@rm -r build
