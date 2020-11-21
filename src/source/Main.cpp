#include "../headers/Main.h"

uint64_t Main::PUSystemMemory = 0;
long Main::MemoryKB = 0;
std::future<int> Main::Install;

int Main::Begin() {
	Logger log("Log");
	glfwSetErrorCallback(glfw_error_callback);
	log.write("Set GLFW error call back.");
	log.write("Attempting to initialise GLFW.");
	if (!glfwInit()) {
		log.write("Could not initialise GLFW", 3);
		return 1;
	}
	log.write("Initialsed GLFW");

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
	log.write("Attempting to read system memory.");
#ifdef _WIN32

	if (GetPhysicallyInstalledSystemMemory(&PUSystemMemory)) {
		MemoryKB = safer_cast<long, uint64_t>(PUSystemMemory);
		log.write("Successfully read system memory");
	}
	else {
		log.write(std::string("Could not read system memory, error:").append(std::to_string(GetLastError())), 2);
	}

#elif defined(__linux__)
	MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);

#elif defined(__APPLE__)
	MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
#endif

	// Create window with graphics context
	log.write("Attempting to create window.");
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Pixel MK 3 Installer", NULL, NULL);
	if (window == NULL) {
		log.write("Could not create window.", 3);
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	log.write("Attempting to initialise GLEW.");
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
		log.write("Failed to initialize OpenGL loader!", 3);
		return 1;
	}
	log.write("Creating ImGUI context.");
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	log.write("Setting colours");
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	log.write("Create bindings");
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	WindowLoop(window, &log);
	log.~Logger();
	return Cleanup(window);
}

void Main::WindowLoop(GLFWwindow* window, Logger* log) {
	log->write("Grabbing io settings.");
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;

	log->write("Setting clear colour");
	ImVec4 clear_colour = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	log->write("Creating file browsers");
	ImGui::FileBrowser MCDir(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_NoModal);
	ImGui::FileBrowser PXMKDir(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_NoModal);

	log->write("Creating fonts");
	ImFontConfig LtlFont_cfg;
	LtlFont_cfg.FontDataOwnedByAtlas = false;
	LtlFont_cfg.FontData = font::NexaSlab;
	LtlFont_cfg.FontDataSize = 77028;

	ImFontConfig StdFont_cfg = LtlFont_cfg;
	ImFontConfig TtlFont_cfg = LtlFont_cfg;

	LtlFont_cfg.SizePixels = 12.5;
	StdFont_cfg.SizePixels = 25;
	TtlFont_cfg.SizePixels = 50;

	log->write("Adding fonts");
	ImFont* LittleFont = io.Fonts->AddFont(&LtlFont_cfg);

	ImFont* stdFont = io.Fonts->AddFont(&StdFont_cfg);

	ImFont* TitleFont = io.Fonts->AddFont(&TtlFont_cfg);

	bool open = true;

	log->write("Getting .minecraft location");
#if defined(_WIN32)
	std::string defaultMCDir = "";
	if (fileHandling::IsDirectory(std::string(getenv("APPDATA")).append("/.minecraft/"))) {
		defaultMCDir = std::string(getenv("APPDATA")).append("/.minecraft/");
	}
	else
		log->write("Failed to find .minecraft location", 1);
	log->write("Setting Java location");
	std::string JavaDir = "C:/Program Files/Java";
#elif defined(__APPLE__)
	std::string defaultMCDir = "~/Library/Application Support/minecraft/";
	std::string JavaDir = "~/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/"
#elif defined(__linux__)
	std::string JavaDir = "usr/bin/java";
	std::string defaultMCDir = "~/.minecraft/";
#endif
	static char bufMCDir[256];
	strcpy(bufMCDir, defaultMCDir.c_str());

	log->write("Identifying all Java directories");
	std::vector<std::filesystem::directory_entry> JavaDirs;
#ifdef _WIN32
	if (fileHandling::IsDirectory(JavaDir)) {
		for (const std::filesystem::directory_entry& p : std::filesystem::directory_iterator(JavaDir)) {
			if (p.is_directory() && (p.path().filename().string().find("jre") != std::string::npos)) {
				JavaDirs.push_back(p);
				log->write("Added Java directory " + p.path().string());
			}
		}
	}
#else
	JavaDirs.push_back(std::filesystem::path(JavaDir));
#endif
	log->write("Identifying amount of Java directories");
	size_t AmtOfJavaDirs = JavaDirs.size();
	log->write("Amount of Java Dirs: " + AmtOfJavaDirs);
	int Chosen = 0;

	static char PixelMKResultDir[256] = "";

	log->write("Checking to see if Forge is installed");
	boost::container::vector<bool> options = { true, true, true, CheckForge(bufMCDir),  (AmtOfJavaDirs <= 0) ? false : true };
	boost::container::vector<std::string> paths;
	int MemoryGB = 8;
	int MaxMemory;

	if (MemoryKB != 0)
		MaxMemory = ((MemoryKB / 1024) / 1024) - 4;
	else
		MaxMemory = 32;

	int Result = -1;

	double Percent = 0;
	double Progress = 0;
	std::string ProgressDesc = "";
	bool EnteredCode = false;
	char code[18] = "";

	log->write("Entering main loop.");
	log->~Logger();
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)));
		ImGui::SetNextWindowPos(ImVec2(0, 0));

		ImGui::Begin("Pixel MK 3 Installer", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::PushFont(TitleFont);
		ImGui::Text("Pixel MK 3 Installer");
		ImGui::PopFont();

		ImGui::PushFont(stdFont);
		if ((MaxMemory > 9 || EnteredCode)) {
			ImGui::Text(".minecraft directory");
			ImGui::InputText("##.McDir", bufMCDir, 256, ImGuiInputTextFlags_AutoSelectAll);
			if (ImGui::Button("Choose Dir##MCDir")) {
				MCDir.Open();
			}

#ifdef _WIN32
			if (AmtOfJavaDirs > 0) {
				ImGui::Text("Choose Java Runtime Environment");
				for (uint8_t i = 0; i < AmtOfJavaDirs; i++) {
					ImGui::RadioButton(JavaDirs.at(i).path().filename().string().c_str(), &Chosen, i);
					if (i != AmtOfJavaDirs - 1) {
						ImGui::SameLine();
					}
				}
			}
			else {
				ImGui::Text("You do not have a Java Runtime Environment. You can download Java here:");
				ImGui::TextURL("java.com", "https://java.com/en/download/manual.jsp");
			}
#endif

			ImGui::Text("Where to place Pixel MK 3?");
			ImGui::InputText("##PixelMKDir", PixelMKResultDir, 256);
			if (ImGui::Button("Choose Dir##PXMKDir")) {
				PXMKDir.Open();
			}

			ImGui::Text("How much memory do you wish to give the modpack (in GB)?");
			ImGui::SliderInt("##Memory", &MemoryGB, 8, MaxMemory);

			ImGui::Checkbox("Do you want the PureBDCraft Resourcepack that pairs with this modpack?", &options[0]);
			ImGui::Checkbox("Do you want the default option settings for this modpack?", &(options[1]));
			ImGui::Checkbox("Do you want the best JVM arguments for performance installed to the modpack?", &options[2]);

			if (!options[3]) {
				ImGui::Text("The correct version of Forge has not been installed. The installer will install this for you.");
			}
			if (Percent == 0.0f) {
				if (strlen(PixelMKResultDir) != 0 && strlen(bufMCDir) != 0) {
					if (ImGui::Button("Install")) {
						Result = -1;
						Install = std::async(CreateProfile::Begin, options, paths, MemoryGB, &Percent, &Progress, &ProgressDesc);
					}
				}
			}
			if (Install.valid()) {
				auto status = Install.wait_for(std::chrono::milliseconds(0));
				if (status == std::future_status::ready) {
					Result = Install.get();
				}
				else {
					ImGui::ProgressBar(Percent);
					if (Progress != 0.0f) {
						ImGui::Text(ProgressDesc.c_str());
						ImGui::ProgressBar(Progress);
					}
				}
			}

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
				ImGui::Text("The install failed due to being unable to download the modpack or install the modpack.");
			}
			else if (Result == 0) {
				ImGui::ProgressBar(1);
				ImGui::Text("Done.");
				Percent = 0.0f;
				Progress = 0.0f;
			}

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
			ImGui::PopFont();
			ImGui::PushFont(LittleFont);
		}
		else {
			ImGui::Text("Your installed memory is less than 13GB.\nYou need at least 9GB to run the modpack and at least 13GB to also run your OS smoothly.");
			if (MaxMemory >= 5) {
				ImGui::Text("If you have been given a passcode to run this modpack then please enter it below.");
				ImGui::InputText("##Code", code, sizeof(code));

				std::string a("5O 78 6C 4d 4b 3E");
				if (!a.compare(code)) {
					MaxMemory = MaxMemory + 4;
					EnteredCode = 1;
				}
			}
			ImGui::PopFont();
			ImGui::PushFont(LittleFont);
			ImGui::Text("If you wish to buy more RAM, you can look at these retailers:");
			ImGui::TextURL("Amazon", "https://www.amazon.co.uk/b/?node=430511031&ref_=Oct_s9_apbd_odnav_hd_bw_bT0akp_0&pf_rd_r=BC0NWPQG7Q1PA4BSVDS3&pf_rd_p=d705626f-64c9-52fa-8b95-df0de1496db3&pf_rd_s=merchandised-search-10&pf_rd_t=BROWSE&pf_rd_i=428655031");
			ImGui::TextURL("Box", "https://www.box.co.uk/memory/sort/1/refine/49019~147657$49019~148264$49019~197745$49019~67690$49019~67704");
			ImGui::TextURL("Ebuyer", "https://www.ebuyer.com/store/Components/cat/Memory---PC");
			ImGui::TextURL("Newegg", "https://www.newegg.com/global/uk-en/p/pl?N=101582542%20500000512%20500001024%20500002048%20500004096%20500008192%20500016384&Order=1");
			ImGui::TextURL("Nova Tech", "https://www.novatech.co.uk/products/components/memory-pc/");
			ImGui::TextURL("Overclockers UK", "https://www.overclockers.co.uk/pc-components/memory");
			ImGui::TextURL("Scan", "https://www.scan.co.uk/shop/computer-hardware/memory-ram/all");
			ImGui::Text("I am not sponsored by any of these");
		}
		ImGui::PopFont();
		ImGui::PushFont(stdFont);

		std::time_t currentTime = std::time(NULL);
		ImGui::Text(std::ctime(&currentTime));

		ImGui::PopFont();
		ImGui::PushFont(LittleFont);

		ImGui::Text("Made by Joe Targett, if you have any problems please contact me @ Joe.Targett@outlook.com");

		ImGui::PopFont();
		ImGui::End();

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
		if (JavaDirs.size() != 0) {
			paths = { PixelMKResultDir, bufMCDir,
	#ifdef _WIN32
				JavaDirs.at(Chosen).path().string().append("/bin/javaw.exe")
	#elif defined(__APPLE__)
				JavaDirs.at(Chosen).path().string()
	#elif defined(__linux__)
				JavaDirs.at(Chosen).path().string()
	#endif
			};
		}
		else {
			paths = { PixelMKResultDir, bufMCDir,"" };
		}
	}
	log->open();
	log->write("Exiting main loop");
}

bool Main::CheckForge(std::string MCDir) {
	if (fileHandling::IsFile(MCDir + "versions/1.12.2-forge-14.23.5.2854/1.12.2-forge-14.23.5.2854.json") &&
		fileHandling::IsFile(MCDir + "versions/1.12.2-forge-14.23.5.2854/1.12.2-forge-14.23.5.2854.jar") ||
		fileHandling::IsFile(MCDir + "versions/1.12.2-forge-14.23.5.2854/1.12.2-forge-14.23.5.2854.json")) {
		return true;
	}
	else {
		return false;
	}
}