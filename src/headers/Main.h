#pragma once
#pragma comment(lib, "crypt32.lib")
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <string>
#include <boost/container/vector.hpp>
#include <thread>
#include <future>
#include <chrono>
#include < fcntl.h >
#include "../vender/imgui/imgui.h"
#include "../vender/imgui/imgui_impl_glfw.h"
#include "../vender/imgui/imgui_impl_opengl3.h"
#include "ImFileBrowser.h"
#include "TextUrl.h"
#include "CreateProfile.h"
#include "Font.h"
#include "Logger.h"
#include "Image.h"

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
	static void WindowLoop(GLFWwindow* window, Logger* Log);
	static int Cleanup(GLFWwindow* window, Logger* log) {
		try {
			log->write("Cleaning up");
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			glfwDestroyWindow(window);
			glfwTerminate();
			return 0;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
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
