#include "../headers/Main.h"
#include <stdio.h>

uint64_t Main::PUSystemMemory = 0;
long Main::MemoryKB = 0;
std::future<int> Main::Install;

int Main::Begin() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

#ifdef _WIN32
	if (GetPhysicallyInstalledSystemMemory(&PUSystemMemory))
		MemoryKB = safer_cast<long, uint64_t>(PUSystemMemory);
	else
		std::cout << GetLastError() << std::endl;

#elif __linux__
	MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);

#elif __APPLE__
	MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
#endif

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Pixel MK 3 Installer", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
	glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
	glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	WindowLoop(window);
	return Cleanup(window);
}

void Main::WindowLoop(GLFWwindow* window) {
	HRESULT hr = S_OK;

	ImVec4 clear_colour = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::FileBrowser MCDir(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_NoModal);
	ImGui::FileBrowser PXMKDir(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_NoModal);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;

	ImFont* stdFont = io.Fonts->AddFontFromFileTTF("NexaSlab.ttf", 25);
	ImFont* TitleFont = io.Fonts->AddFontFromFileTTF("NexaSlab.ttf", 50);

	bool open = true;

	std::string defaultMCDir = getenv("APPDATA");
	defaultMCDir += "\\.minecraft\\";
	static char bufMCDir[256];
	strcpy(bufMCDir, defaultMCDir.c_str());

	std::string JavaDir = "C:/Program Files/Java";
	std::vector<std::filesystem::directory_entry> JavaDirs;
	for (auto& p : std::filesystem::directory_iterator(JavaDir))
		if (p.is_directory() && (p.path().filename().string().find("jre") != std::string::npos))
			JavaDirs.push_back(p);
	size_t AmtOfJavaDirs = JavaDirs.size();
	int Chosen = 0;

	static char PixelMKResultDir[256] = "";

	bool options[4];
	int MemoryGB = 8;
	int MaxMemory;
	if (MemoryKB != 0)
		MaxMemory = (MemoryKB / 1024) / 1024;
	else
		MaxMemory = 32;

	options[3] = CheckForge(bufMCDir);

	int Result = 5;

	std::promise<float> result_promise;
	std::future<float> PercentageGetter = result_promise.get_future();
	float modifier = 0;
	std::atomic<float> Percent = 0;
	std::atomic<float> Progress = 0;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int width;
		int height;
		glfwGetWindowSize(window, &width, &height);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)));
		ImGui::SetNextWindowPos(ImVec2(0, 0));

		ImGui::Begin("Pixel MK 3 Installer", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::PushFont(TitleFont);
		ImGui::Text("Pixel MK 3 Installer");
		ImGui::PopFont();

		ImGui::PushFont(stdFont);

		ImGui::Text(".minecraft directory");
		ImGui::InputText("##.McDir", bufMCDir, 256, ImGuiInputTextFlags_AutoSelectAll);
		if (ImGui::Button("Choose Dir##MCDir")) {
			MCDir.Open();
		}

		ImGui::Text("Choose Java Runtime Environment");
		for (uint8_t i = 0; i < AmtOfJavaDirs; i++) {
			ImGui::RadioButton(JavaDirs.at(i).path().filename().string().c_str(), &Chosen, i);
			if (i != AmtOfJavaDirs - 1) {
				ImGui::SameLine();
			}
		}

		ImGui::Text("Where to place Pixel MK 3?");
		ImGui::InputText("##PixelMKDir", PixelMKResultDir, 256);
		if (ImGui::Button("Choose Dir##PXMKDir")) {
			PXMKDir.Open();
		}

		ImGui::Text("How much memory do you wish to give the modpack (in GB)?");
		ImGui::SliderInt("##Memory", &MemoryGB, 8, MaxMemory);

		ImGui::Checkbox("Do you want the PureBDCraft Resourcepack that pairs with this modpack?", &options[0]);
		ImGui::Checkbox("Do you want the default option settings for this modpack?", &options[1]);
		ImGui::Checkbox("Do you want the best JVM arguments for performance installed to the modpack?", &options[2]);

		if (!options[3]) {
			ImGui::Text("The correct version of Forge has not been installed. The installer will install this for you.");
		}

		if (strlen(PixelMKResultDir) != 0 && strlen(bufMCDir) != 0) {
			if (ImGui::Button("Install")) {
				Install = std::async(CreateProfile::Begin, options, PixelMKResultDir, bufMCDir, MemoryGB, JavaDirs.at(Chosen).path().string().append("/bin/javaw.exe"), &Percent, &Progress);
			}
		}
		if (Install.valid()) {
			auto status = Install.wait_for(std::chrono::milliseconds(0));
			if (status == std::future_status::ready) {
				Result = Install.get();
				if (Result == 4) {
					ImGui::Text("The install failed due to forge being unable to install. Are you connected to Wi-Fi?");
				}
				else if (Result == 3) {
					ImGui::Text("The install failed due to the Pixel MK Directory being unable to be created. Did you enter the path correctly?");
				}
				else if (Result == 2) {
					ImGui::Text("The install failed due to an issue with writing to launcher_profiles.json. Did you enter the /.minecraft directory correctly?");
				}
				else if (Result == 1) {
					ImGui::Text("The install failed due to being unable to download the modpack.");
				}
				else if (Result == 0) {
					ImGui::ProgressBar(1);
					ImGui::Text("Done.");
				}
			}
			else {
				ImGui::ProgressBar(Percent.load());
				if (Progress.load() != 0) {
					ImGui::ProgressBar(Progress.load());
				}
			}
		}

		std::time_t currentTime = std::time(NULL);
		ImGui::Text(std::ctime(&currentTime));

		ImGui::PopFont();
		ImGui::End();

		MCDir.Display();

		if (MCDir.HasSelected())
		{
			strcpy(bufMCDir, MCDir.GetSelected().string().c_str());
			MCDir.ClearSelected();
		}

		PXMKDir.Display();
		if (PXMKDir.HasSelected())
		{
			strcpy(PixelMKResultDir, PXMKDir.GetSelected().string().c_str());
			PXMKDir.ClearSelected();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		options[3] = CheckForge(bufMCDir);
	}
	delete options;
}

bool Main::CheckForge(std::string MCDir) {
	if (std::filesystem::exists(MCDir + "versions\\1.12.2-forge-14.23.5.2854\\1.12.2-forge-14.23.5.2854.json") && std::filesystem::exists(MCDir + "versions\\1.12.2-forge-14.23.5.2854\\1.12.2-forge-14.23.5.2854.jar") || std::filesystem::exists(MCDir + "versions\\1.12.2-forge-14.23.5.2854\\1.12.2-forge-14.23.5.2854.json")) {
		return true;
	}
	else {
		return false;
	}
}