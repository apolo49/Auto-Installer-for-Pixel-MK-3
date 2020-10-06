#pragma once
#pragma comment(lib, "crypt32.lib")
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <atomic>
#include "../vender/imgui/imgui.h"
#include "../vender/imgui/imgui_impl_glfw.h"
#include "../vender/imgui/imgui_impl_opengl3.h"
#include "ImFileBrowser.h"
#include "CreateProfile.h"

#ifdef _WIN32
#include <sysinfoapi.h>

#elif __linux__
#include <unistd.h>
MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);

#elif __APPLE__
#include <unistd.h>
MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
#endif

class Main
{
public:
	static std::future<int> Install;
	static uint64_t PUSystemMemory;
	static long MemoryKB;

	template<typename T, typename U>
	static T safer_cast(const U& from) {
		T to;
		memcpy(&to, &from, (sizeof(T) > sizeof(U) ? sizeof(U) : sizeof(T)));
		return to;
	}

	static int Begin();
	static void WindowLoop(GLFWwindow* window);
	static int Cleanup(GLFWwindow* window) {
		try {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			glfwDestroyWindow(window);
			glfwTerminate();

			return 0;
		}
		catch (std::exception&) {
			return 1;
		}
	}

private:
	static void glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}

	static bool CheckForge(std::string MCDir);
};
