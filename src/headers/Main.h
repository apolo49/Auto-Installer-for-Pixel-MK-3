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
#include <fcntl.h>

#include "../vender/imgui/imgui.h"
#include "../vender/imgui/imgui_impl_glfw.h"
#include "../vender/imgui/imgui_impl_opengl3.h"

#include "ImFileBrowser.h"
#include "TextUrl.h"
#include "CreateProfile.h"
#include "Font.h"
#include "Logger.h"
#include "Image.h"
#include "Mods.h"

#ifdef _WIN32
#include <sysinfoapi.h>

#elif __linux__
#include <unistd.h>
#include <pwd.h>
#include <sys/sysinfo.h>

#elif __APPLE__
#include <unistd.h>
#endif

class Main
{
public:
	static std::future<int> Install;
	static uint64_t PUSystemMemory;
	static long MemoryKB;
	static ImVec4 clear_colour;
	static std::vector<ImFont*> Fonts;
	static Logger* log;
	static GLFWwindow* window;

	template<typename T, typename U>
	static T safer_cast(const U& from) {
		T to;
		memcpy(&to, &from, (sizeof(T) > sizeof(U) ? sizeof(U) : sizeof(T)));
		return to;
	}

	static int Begin();
	static void WindowLoop();

	static int Cleanup() {
		try {
			log->write("Cleaning up.");
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
	static void BeginFrame();
	static void ListOfModsWindow(bool& ModWindow);
	static void TitleScreen(bool& MainWindow, bool& ModWindow);
	static void NotEnoughMemWindow(int& MaxMemory, char* code, bool& UnsafeMode, bool& MainWindow);
	static void MainScreen(char* bufMCDir, ImGui::FileBrowser& MCDir, size_t& AmtOfJavaDirs, std::vector<std::filesystem::directory_entry>& JavaDirs,
		int& Chosen, char* PixelMKResultDir, ImGui::FileBrowser& PXMKDir, bool& UnsafeMode, int& MemoryGB, int& MaxMemory, double& Percent,
		double& Progress, uint8_t& Result, boost::container::vector<bool>& options, std::string& ProgressDesc,
		boost::container::vector<std::string>& paths, bool& MainWindow);
	static bool CheckForge(std::string MCDir);
	static void CreateFonts();
	static void EndFrame();
	static void GetMemory();
	static std::string MinecraftDirLocator();
	static std::vector<std::filesystem::directory_entry> JavaDirLocator();
};
